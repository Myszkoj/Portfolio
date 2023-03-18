#include "..//include/fqs_Tests.h"
#include <iostream>
#include <random>
#include <chrono>


namespace fqs
{
	static uint64_t	perform_test(	Organizer&			organizer,
									dpl::ThreadPool&	threadPool)
	{
		uint64_t ms = 0;
			
		const dpl::Range<uint32_t>	DISTANCE_RANGE(1, organizer.settings().sectionsPerQ * organizer.settings().sectionLength);

		std::random_device rng;
		std::uniform_int_distribution<uint32_t> VISIBLE_Q(0, organizer.settings().numQueues()-1);
		std::uniform_int_distribution<uint32_t> DIST(DISTANCE_RANGE.min(), DISTANCE_RANGE.max());

		for(uint64_t index = 0; index < organizer.settings().numQueuers(); ++index)
		{
			organizer.get_queuers()[index].update_with_cm((uint32_t)index, VISIBLE_Q(rng), DIST(rng));
		}
	
		{ //ASSIGNER UPDATE
			auto start	= std::chrono::steady_clock::now();	
			organizer.update(threadPool);
			auto end	= std::chrono::steady_clock::now();

			ms += std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
		}
		
		return ms;
	}

	void			test_queues(	Organizer&			organizer,
									const uint64_t		NUM_TESTS,
									const uint32_t		TOTAL_NUM_AGENTS,
									const uint32_t		NUM_QUEUES,
									const uint32_t		NUM_SECTIONS_PER_QUEUE,
									const float			SECTION_LENGTH)
	{
		dpl::ThreadPool threadPool;

		fqs::Settings	settings(TOTAL_NUM_AGENTS, NUM_QUEUES);
						settings.sectionsPerQ = NUM_SECTIONS_PER_QUEUE;
						settings.sectionLength = (uint32_t)(100 * SECTION_LENGTH);

		organizer.initialize(settings);

		uint64_t timeTotal = 0;
		for(uint64_t testID = 0; testID < NUM_TESTS; ++testID)
		{
			std::cout << "testing..." << std::endl;
			const auto TIME = perform_test(organizer, threadPool);
			std::cout << "time: " << TIME << "ms" << std::endl;

			timeTotal += TIME;
		}

		std::cout << "avr. time: " << timeTotal / NUM_TESTS  << "ms" << std::endl;
	}
}