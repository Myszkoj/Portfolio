#include "..//include/upf_SpatialDivision.h"
#include <unordered_map>

#pragma warning( disable : 26451 ) // arithmetic overflow

// public functions
namespace upf
{
	void		SpatialDivision::initialize(	const Series&		SERIES,
												const bool			LOG_PAIR_GENERATION)
	{
		series				= SERIES;
		logPairGeneration	= LOG_PAIR_GENERATION;

		auto* block = blocks->data();
		for(uint8_t v = 0; v < 2; ++v)
		{
			for(uint8_t hy = 0; hy < 2; ++hy)
			{
				for(uint8_t hx = 0; hx < 2; ++hx)
				{
					(block++)->initialize(Voxel(hx?1:0, hy?1:0, v?1:0), series);
				}
			}
		}
	}

	void		SpatialDivision::update(		dpl::ParallelPhase*	threadPool,
												const TestPair&		TEST_PAIR)
	{
		update_boxes(threadPool);
		update_blocks(threadPool);
		find_pairs(threadPool, TEST_PAIR);
	}
}

// private functions
namespace upf
{
	void		SpatialDivision::update_boxes(	dpl::ParallelPhase*	threadPool)
	{
		auto update_range = [&](const dpl::IndexRange<uint32_t> RANGE)
		{
			char*		boxPtr			= series().objects() + series().offset();
			const auto  BOX_SIZE		= series().boxSize();
			const auto	HALF_BOX_SIZE	= BOX_SIZE / 2.f;

			RANGE.for_each([&](const uint32_t OBJ_ID)
			{
				auto&	box = *reinterpret_cast<Box*>(boxPtr + OBJ_ID * series().stride());
						box.update_voxel(BOX_SIZE, HALF_BOX_SIZE);
			});
		};

		if(threadPool)
		{
			dpl::IndexRange<>(0, series().size()).for_each_split(threadPool->numJobs(), [&](auto range)
			{
				threadPool->add_task(range.size(), [&, range]()
				{
					update_range(range);
				});
			});

			threadPool->start();
		}
		else
		{
			update_range(dpl::IndexRange<>(0, series().size()));
		}
	}

	void		SpatialDivision::update_blocks(	dpl::ParallelPhase*	threadPool)
	{
		if(threadPool)
		{
			for(auto& iBlock : *blocks)
			{
				threadPool->add_task(1000, [&]()
				{
					iBlock.update(series);
				});
			}

			threadPool->start();
		}
		else
		{
			for(auto& iBlock : *blocks)
			{
				iBlock.update(series);
			}
		}
		
	}

	void		SpatialDivision::find_pairs(	dpl::ParallelPhase*	threadPool,
												const TestPair&		TEST_PAIR)
	{
		std::atomic_uint64_t numTotalPairs	= 0;

		auto find_pairs_in_range = [&](const Block& iBLOCK, const dpl::IndexRange<uint32_t> range)
		{
			uint64_t numPairsInRange = 0;

			const auto* PROXIES = iBLOCK.get();
			auto		it		= iBLOCK.begin(range.begin(), range.end());
			const auto	END		= iBLOCK.end(range.end());

			auto test_sublist = [&](const Block::Proxy& PROXY_A, uint32_t aid, uint32_t bid)
			{
				while(bid != Box::INVALID_ID)
				{
					const auto& PROXY_B = PROXIES[bid];
					if(PROXY_A.can_pair_with(PROXY_B))
					{
						TEST_PAIR(	series().objects() + aid * series().stride(), 
									series().objects() + bid * series().stride());

						++numPairsInRange;
					}

					bid = PROXY_B.nextProxyID;
				}
			};

			while(it != END)
			{
				uint32_t aid = it->second.mainIndex;

				while(aid != Box::INVALID_ID)
				{
					const auto& PROXY_A = PROXIES[aid];

					test_sublist(PROXY_A, aid, PROXY_A.nextProxyID);
					test_sublist(PROXY_A, aid, it->second.adjacentIndex);
							
					aid = PROXY_A.nextProxyID;
				}

				++it;
			}

			numTotalPairs += numPairsInRange;
		};

		if(threadPool)
		{
			for(const auto& iBLOCK : blocks())
			{
				dpl::IndexRange(0u, iBLOCK.numBuckets()).for_each_split(threadPool->numJobs(), [&](auto range)
				{
					threadPool->add_task(range.size(), [&, range]()
					{
						find_pairs_in_range(iBLOCK, range);
					});
				});

				threadPool->start();
			}
		}
		else
		{
			for(const auto& iBLOCK : blocks())
			{
				find_pairs_in_range(iBLOCK, dpl::IndexRange(0u, iBLOCK.numBuckets()));
			}
		}

		if(logPairGeneration) dpl::Logger::ref().push_info("Number of generated PCP: %d", numTotalPairs.load());
	}
}