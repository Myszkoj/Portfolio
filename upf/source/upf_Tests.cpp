#include "..//include/upf_Tests.h"
#include "..//include/upf_SpatialDivision.h"
#include <iostream>

#define USE_CORE_THREAD_POOL

namespace upf
{
	class TestObject : public upf::Box
	{

	};

	class PairTester
	{
	private: // data
		std::atomic<uint64_t>	m_pairs = 0;

	public: // functions
		inline void		reset_counter()
		{
			m_pairs.store(0);
		}

		inline uint64_t	get_numPairs() const
		{
			return m_pairs.load();
		}

		inline void		on_test_pair(	void* objA,	
										void* objB)
		{
			++m_pairs;
		}
	};


#ifdef USE_CORE_THREAD_POOL

#include <Core_Thread.h>

	class TestSeries	: public Series
						, public PairTester
	{
	private: // data
		Core::ThreadPool threadPool;

	public: // lifecycle
		/* CTOR */		TestSeries(		TestObject*						objects,
										const uint32_t					NUM_OBJECTS,
										const HVSize&					BOX_SCALE)
			: Series(objects, NUM_OBJECTS, BOX_SCALE, std::thread::hardware_concurrency(), std::bind(&PairTester::on_test_pair, this, std::placeholders::_1, std::placeholders::_2))
			
		{
			
		}

	public: // interface
		virtual void	on_stage_begin(	const Stage						STAGE){}

		virtual void	add_task(		const std::function<void()>&	TASK)
		{
			threadPool.add_task(TASK);
		}

		virtual void	on_stage_end(	const Stage						STAGE)
		{
			threadPool.wait();
		}
	};

#else
	class TestSeries	: public Series
						, public PairTester
	{
	public: // lifecycle
		/* CTOR */		TestSeries(	TestObject*		objects,
									const uint32_t	NUM_OBJECTS,
									const HVSize&	BOX_SCALE)
			: Series(objects, NUM_OBJECTS, BOX_SCALE, 1, std::bind(&PairTester::on_test_pair, this, std::placeholders::_1, std::placeholders::_2))
			
		{
			
		}
	};

#endif // USE_CORE_THREAD_POOL

	void test_random_objects(	const uint64_t									NUM_TESTS,
								const uint64_t									NUM_OBJECTS,
								const upf::HVSize&								BOX_SIZE,
								const std::uniform_real_distribution<float>&	HPOSITION_RANGE,
								const std::uniform_real_distribution<float>&	VPOSITION_RANGE)
	{
		static std::random_device	rng;
		Core::ThreadPool			threadPool;
		upf::SpatialDivision		division;
		std::vector<TestObject>		objects(NUM_OBJECTS);

		auto series	= std::make_shared<TestSeries>(objects.data(), (uint32_t)objects.size(), BOX_SIZE);

		for(auto& iObject : objects)
		{
			iObject.center.h.x	= HPOSITION_RANGE(rng);
			iObject.center.h.y	= HPOSITION_RANGE(rng);
			iObject.center.v	= VPOSITION_RANGE(rng);
		}

		uint64_t numPairsTotal	= 0;
		uint64_t timeTotal		= 0;

		for(uint64_t testID = 0; testID < NUM_TESTS; ++testID)
		{
			std::cout << "testing..." << std::endl;

			series->reset_counter();

			auto start	= std::chrono::steady_clock::now();	
			division.update(series);
			auto end	= std::chrono::steady_clock::now();

			const uint64_t TIME = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
		
			for(uint64_t index = 0; index < objects.size(); ++index)
			{
				auto&	object = objects[index];
						object.center.h.x	= HPOSITION_RANGE(rng);
						object.center.h.y	= HPOSITION_RANGE(rng);
						object.center.v		= VPOSITION_RANGE(rng);
			}

			const auto NUM_PAIRS = series->get_numPairs();

			std::cout << "Time:         " << TIME << "ms" << std::endl;
			std::cout << "Pairs:        " << NUM_PAIRS << std::endl;

			timeTotal		+= TIME;
			numPairsTotal	+= NUM_PAIRS;
		}

		std::cout << "avr. time: " << timeTotal / NUM_TESTS  << "ms" << std::endl;
		std::cout << "avr. pairs: " << numPairsTotal / NUM_TESTS << std::endl;
	}

	void test_moving_objects(	const uint64_t									NUM_TESTS,
								const uint64_t									NUM_OBJECTS,
								const upf::HVSize&								BOX_SIZE,
								const std::uniform_real_distribution<float>&	HPOSITION_RANGE,
								const std::uniform_real_distribution<float>&	VPOSITION_RANGE,
								const float										DELTA)
	{
		static std::random_device	rng;
		Core::ThreadPool			threadPool;
		upf::SpatialDivision		division;
		std::vector<TestObject>		objects(NUM_OBJECTS);

		auto series	= std::make_shared<TestSeries>(objects.data(), (uint32_t)objects.size(), BOX_SIZE);

		for(auto& iObject : objects)
		{
			iObject.center.h.x	= HPOSITION_RANGE(rng);
			iObject.center.h.y	= HPOSITION_RANGE(rng);
			iObject.center.v	= VPOSITION_RANGE(rng);
		}

		uint64_t numPairsTotal	= 0;
		uint64_t timeTotal		= 0;

		for(uint64_t testID = 0; testID < NUM_TESTS; ++testID)
		{
			std::cout << "testing..." << std::endl;

			series->reset_counter();

			auto start	= std::chrono::steady_clock::now();	
			division.update(series);
			auto end	= std::chrono::steady_clock::now();

			const uint64_t TIME = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
		
			for(uint64_t index = 0; index < objects.size(); ++index)
			{
				glm::vec2 direction(HPOSITION_RANGE(rng), HPOSITION_RANGE(rng));

				if(const float LENGTH = glm::length(direction))
				{
					auto&	object = objects[index];
							object.center.h += direction * (DELTA / LENGTH);
				}
			}

			const auto NUM_PAIRS = series->get_numPairs();

			std::cout << "Time:         " << TIME << "ms" << std::endl;
			std::cout << "Pairs:        " << NUM_PAIRS << std::endl;

			timeTotal		+= TIME;
			numPairsTotal	+= NUM_PAIRS;
		}

		std::cout << "avr. time: " << timeTotal / NUM_TESTS  << "ms" << std::endl;
		std::cout << "avr. pairs: " << numPairsTotal / NUM_TESTS << std::endl;
	}

	void test_uniform_objects(	const uint64_t									NUM_OBJECTS_PER_LINE)
	{
		static std::random_device	rng;
		Core::ThreadPool			threadPool;
		upf::SpatialDivision		division;
		std::vector<TestObject>		objects(NUM_OBJECTS_PER_LINE * NUM_OBJECTS_PER_LINE);
		const upf::HVSize			BOX_SIZE(2.f, 2.f);
	
		auto series	= std::make_shared<TestSeries>(objects.data(), (uint32_t)objects.size(), BOX_SIZE);

		for(uint64_t y = 0; y < NUM_OBJECTS_PER_LINE; ++y)
		{
			for(uint64_t x = 0; x < NUM_OBJECTS_PER_LINE; ++x)
			{
				const uint64_t OBJECT_ID = y * NUM_OBJECTS_PER_LINE + x;

				auto&	object = objects[OBJECT_ID];
						object.center.h.x	= -(x * BOX_SIZE.horizontal) + BOX_SIZE.horizontal * 3.f / 4.f;
						object.center.h.y	= -(y * BOX_SIZE.horizontal) + BOX_SIZE.horizontal * 3.f / 4.f;
						//object.center.h.x	= (x * BOX_SIZE.horizontal) + BOX_SIZE.horizontal / 4.f;
						//object.center.h.y	= (y * BOX_SIZE.horizontal) + BOX_SIZE.horizontal / 4.f;
						object.center.v		= 0.f; //POSITION_RANGE(rng);
			}
		}

		std::cout << "testing..." << std::endl;

		auto start	= std::chrono::steady_clock::now();	
		division.update(series);
		auto end	= std::chrono::steady_clock::now();

		const auto	TIME		= std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();	
		const auto	NUM_PAIRS	= series->get_numPairs();

		std::cout << "Time:         " << TIME << "ms" << std::endl;
		std::cout << "Est. Pairs:   " << objects.size() * 3 << std::endl;
		std::cout << "Pairs:        " << NUM_PAIRS << std::endl;
	}
}