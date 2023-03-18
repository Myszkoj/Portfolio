#include "..//include/fqs_Organizer.h"


// public functions
namespace fqs
{
	void	Organizer::initialize(		const Settings&		NEW_SETTINGS)
	{
		if(settings().numQueuers() != NEW_SETTINGS.numQueuers())
		{
			queuers.resize(NEW_SETTINGS.numQueuers());
			//queuers = std::make_unique<Queuer[]>(NEW_SETTINGS.numQueuers());
		}

		const auto NEW_NUM_SECTIONS = NEW_SETTINGS.get_numSections();

		if(settings().get_numSections() != NEW_NUM_SECTIONS)
		{
			sections.resize(NEW_NUM_SECTIONS);
			//sections = std::make_unique<Section[]>(NEW_NUM_SECTIONS);
		}

		settings = NEW_SETTINGS;
	}

	void	Organizer::update(			dpl::ThreadPool&	threadPool)
	{
		// This process can theoretically be offloaded to the GPU
		/*
			1) Instead of doing clear_section, we can simply transfer "clear" buffer.
			2) Compile single shader that performs insert_queuers, sort_sections, or update_queues based on the supplied uniform.
			3) Transfer queuers to the GPU buffer.
			4) Transfer sections to the GPU buffer.
			5) Run shader in insert_queuers mode.
			6) Run shader in sort_sections mode.
			7) Run shader in update_queues mode.
		*/
		const uint32_t NUM_SPLITS = (uint32_t)threadPool.get_numWorkers();
		clear_sections(threadPool, NUM_SPLITS);
		insert_queuers(threadPool, NUM_SPLITS);
		sort_sections(threadPool, NUM_SPLITS);
		update_queues(threadPool, NUM_SPLITS);
	}

	void	Organizer::update(			dpl::ParallelPhase*	threadPool)
	{
		clear_sections(threadPool);
		insert_queuers(threadPool);
		sort_sections(threadPool);
		update_queues(threadPool);
	}
}

// internal functions
namespace fqs
{
	void	Organizer::sort_section(	Section&			section)
	{
		Queuer		newOrder; // Dummy queuer that represents beginning of this section.
		uint32_t	currentQueuerID = section.firstQueuerID.load(std::memory_order_relaxed);

		while(currentQueuerID != Queuer::INVALID_ID)
		{
			Queuer& current	= queuers[currentQueuerID];
			Queuer* target	= &newOrder;

			while(target->m_nextID != Queuer::INVALID_ID)
			{
				Queuer& next = queuers[target->m_nextID];
				if(next.get_distance_cm() > current.get_distance_cm()) break;
				if(next.get_distance_cm() == current.get_distance_cm()) // special case
				{
					if(next.m_uniqueID > current.m_uniqueID)
						break;
				}
				target = &next;
			}

			const uint32_t NEXT_UNSORTED_MEMBER_ID = current.m_nextID;
			current.m_nextID = target->m_nextID;
			target->m_nextID = currentQueuerID;
			currentQueuerID = NEXT_UNSORTED_MEMBER_ID;
		}

		section.firstQueuerID.store(newOrder.m_nextID, std::memory_order_relaxed);
	}

	void	Organizer::update_queue(	const uint32_t		QID)
	{
		float totalValue = 0.f;

		for(uint32_t sectionID = 0; sectionID < settings().sectionsPerQ; ++sectionID)
		{
			const auto& SECTION = get_section(QID, sectionID);

			uint32_t currentQueuerID = SECTION.firstQueuerID.load(std::memory_order_relaxed);
			while(currentQueuerID != Queuer::INVALID_ID)
			{
				Queuer&	current = queuers[currentQueuerID];
				totalValue		+= current.m_value;
				current.m_value = totalValue;
				currentQueuerID = current.m_nextID;
			}
		}

		/*
		if(gCounter >= 1000 && QID == 4) // DEBUG
		{
			std::lock_guard lock(gMTX);

			dpl::Logger::ref().push_warning("### Queue[%d] ###", QID);

			uint32_t	currentDistance		= 0;
			uint32_t	duplicates			= 0;
			Queuer*		first				= nullptr;
			uint32_t	firstSectionID		= 0;

			uint32_t	memberID = 0;
			for(uint32_t sectionID = 0; sectionID < settings().sectionsPerQ; ++sectionID)
			{
				const auto& SECTION = get_section(QID, sectionID);
				uint32_t currentQueuerID = SECTION.firstQueuerID;
				while(currentQueuerID != Queuer::INVALID_ID)
				{
					Queuer& current	= queuers[currentQueuerID];

					if(current.m_dist != currentDistance)
					{
						first = &current;
						firstSectionID	= sectionID;
						currentDistance = current.m_dist;
						duplicates		= 0;
					}
					else
					{
						if(duplicates == 0)
						{
							dpl::Logger::ref().push_info("Member[%d, %d]: distance: %d uniqueID: %d", memberID-1, firstSectionID, first->m_dist, first->m_uniqueID);
						}

						++duplicates;
						dpl::Logger::ref().push_info("Member[%d, %d]: distance: %d uniqueID: %d", memberID, sectionID, current.m_dist, current.m_uniqueID);
					}

					currentQueuerID = current.m_nextID;
					++memberID;
				}
			}

			dpl::Logger::ref().push_info("");
		}*/
	}
}

// standard threadpool stages
namespace fqs
{
	void	Organizer::clear_sections(	dpl::ThreadPool&	threadPool,
										const uint32_t		NUM_SPLITS)
	{
		dpl::IndexRange<>(0, settings().get_numSections()).for_each_split(NUM_SPLITS, [&](auto range)
		{
			threadPool.add_task([&, range]()//<-- Pass range by value!
			{
				range.for_each([&](const uint32_t INDEX)
				{
					sections[INDEX].clear();
				});
			});
		});

		threadPool.wait();
	}

	void	Organizer::insert_queuers(	dpl::ThreadPool&	threadPool,
										const uint32_t		NUM_SPLITS)
	{
		dpl::IndexRange<>(0, settings().numQueuers()).for_each_split(NUM_SPLITS, [&](auto range)
		{
			threadPool.add_task([&, range]()//<-- Pass range by value!
			{
				range.for_each([&](const uint32_t QUEUER_ID)
				{
					auto& queuer = queuers[QUEUER_ID];
					if(queuer.m_QID < settings().numQueues)
					{
						auto& section = get_section(queuer.m_QID, calculate_sectionID(queuer.get_distance_cm()));
						queuer.m_nextID = section.firstQueuerID.exchange(QUEUER_ID);
					}
				});
			});
		});

		threadPool.wait();
	}

	void	Organizer::sort_sections(	dpl::ThreadPool&	threadPool,
										const uint32_t		NUM_SPLITS)
	{
		dpl::IndexRange<>(0, settings().get_numSections()).for_each_split(NUM_SPLITS, [&](auto range)
		{
			threadPool.add_task([&, range]()//<-- Pass range by value!
			{
				range.for_each([&](const uint32_t INDEX)
				{
					sort_section(sections[INDEX]);
				});
			});
		});

		threadPool.wait();
	}

	void	Organizer::update_queues(	dpl::ThreadPool&	threadPool,
										const uint32_t		NUM_SPLITS)
	{
		dpl::IndexRange<>(0, settings().numQueues()).for_each_split(NUM_SPLITS, [&](auto range)
		{
			threadPool.add_task([&, range]()//<-- Pass range by value!
			{
				range.for_each([&](const uint32_t QID)
				{
					update_queue(QID);
				});
			});
		});

		// OLD SOLUTION
		/*
		const uint32_t			NUM_WORKERS = (uint32_t)threadpool.get_numWorkers();
		std::vector<uint32_t>	workers(NUM_WORKERS);
		std::iota (workers.begin(), workers.end(), 0u); // Fill with 0, 1, ..., NUM_WORKERS-1.

		for(const auto& iWORKER_ID : workers)
		{
			add_task([&]()
			{
				// If X = NUM_WORKERS, update every X queue starting from the iWORKER_ID.
				for(uint32_t qid = iWORKER_ID; qid < settings().numQueues; qid += NUM_WORKERS)
				{
					update_queue(qid, organizer);
				}
			});
		}
		*/

		threadPool.wait();
	}
}

// parallel phase stages
namespace fqs
{
	void	Organizer::clear_sections(	dpl::ParallelPhase*	threadPool)
	{
		if(threadPool)
		{
			dpl::IndexRange<>(0, settings().get_numSections()).for_each_split(threadPool->numJobs(), [&](auto range)
			{
				threadPool->add_task(10, [&, range]()//<-- Pass range by value!
				{
					range.for_each([&](const uint32_t INDEX)
					{
						sections[INDEX].clear();
					});
				});
			});

			threadPool->start();
		}
		else
		{
			dpl::IndexRange<>(0, settings().get_numSections()).for_each([&](const uint32_t INDEX)
			{
				sections[INDEX].clear();
			});
		}
	}

	void	Organizer::insert_queuers(	dpl::ParallelPhase*	threadPool)
	{
		auto perform = [&](const dpl::IndexRange<uint32_t> RANGE)
		{
			RANGE.for_each([&](const uint32_t QUEUER_ID)
			{
				auto& queuer = queuers[QUEUER_ID];
				if(queuer.m_QID < settings().numQueues)
				{
					auto& section = get_section(queuer.m_QID, calculate_sectionID(queuer.get_distance_cm()));
					queuer.m_nextID = section.firstQueuerID.exchange(QUEUER_ID);
					++section.numQueuers;
				}
			});
		};

		if(threadPool)
		{
			dpl::IndexRange<>(0, settings().numQueuers()).for_each_split(threadPool->numJobs(), [&](auto range)
			{
				threadPool->add_task(10, [&, range]()//<-- Pass range by value!
				{
					perform(range);
				});
			});

			threadPool->start();
		}
		else
		{
			perform(dpl::IndexRange<>(0, settings().numQueuers()));
		}
	}

	void	Organizer::sort_sections(	dpl::ParallelPhase*	threadPool)
	{
		const uint32_t NUM_SECTIONS = settings().get_numSections();
		for(uint32_t index = 0; index < NUM_SECTIONS; ++index)
		{
			Section* section = sections.data() + index;
			if(const uint32_t NUM_QUEUERS = section->numQueuers.load(std::memory_order_relaxed))
			{
				if(threadPool)
				{
					threadPool->add_task(NUM_QUEUERS, [&, section]()//<-- Pass section ptr by value!
					{
						sort_section(*section);
					});
				}
				else
				{
					sort_section(*section);
				}
			}
		}

		if(threadPool) threadPool->start();
	}

	void	Organizer::update_queues(	dpl::ParallelPhase*	threadPool)
	{
		for(uint32_t qid = 0; qid < settings().numQueues(); ++qid)
		{
			if(threadPool)
			{
				threadPool->add_task(settings().sectionsPerQ, [&, qid]()//<-- Pass range by value!
				{
					update_queue(qid);
				});
			}
			else
			{
				update_queue(qid);
			}
		}

		if(threadPool) threadPool->start();
	}
}