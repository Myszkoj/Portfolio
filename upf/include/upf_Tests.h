#pragma once


#include <random>
#include "upf_HV.h"


namespace upf
{
	void test_random_objects(	const uint64_t									NUM_TESTS,
								const uint64_t									NUM_OBJECTS,
								const upf::HVSize&								BOX_SIZE,
								const std::uniform_real_distribution<float>&	HPOSITION_RANGE,
								const std::uniform_real_distribution<float>&	VPOSITION_RANGE);

	void test_moving_objects(	const uint64_t									NUM_TESTS,
								const uint64_t									NUM_OBJECTS,
								const upf::HVSize&								BOX_SIZE,
								const std::uniform_real_distribution<float>&	HPOSITION_RANGE,
								const std::uniform_real_distribution<float>&	VPOSITION_RANGE,
								const float										DELTA);

	void test_uniform_objects(	const uint64_t									NUM_OBJECTS_PER_LINE);
}