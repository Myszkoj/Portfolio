#pragma once

#include <stdint.h>
#include "fqs_Organizer.h"

namespace fqs
{
	void test_queues(	Organizer&		organizer,
						const uint64_t	NUM_TESTS				= 100,
						const uint32_t	TOTAL_NUM_AGENTS		= 1000000,
						const uint32_t	NUM_QUEUES				= 1600,
						const uint32_t	NUM_SECTIONS_PER_QUEUE	= 32,
						const float		SECTION_LENGTH			= 2.f);
}