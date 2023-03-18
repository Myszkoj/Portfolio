#pragma once


#include "fqs_utilities.h"


namespace fqs
{
	class Settings
	{
	public: // constants
		static const uint32_t DEFAULT_NUM_SECTIONS_PER_QUEUE	= 16;
		static const uint32_t DEFAULT_SECTION_LENGTH_IN_CM		= 400;

		using SectionsPerQueue	= dpl::RangedValue<uint32_t, 1, 64, DEFAULT_NUM_SECTIONS_PER_QUEUE>;
		using SectionLengthCM	= dpl::RangedValue<uint32_t, 100, 10000, DEFAULT_SECTION_LENGTH_IN_CM>;

	public: // data
		dpl::ReadOnly<uint32_t,	Settings>	numQueuers;
		dpl::ReadOnly<uint32_t,	Settings>	numQueues;
		SectionsPerQueue					sectionsPerQ;
		SectionLengthCM						sectionLength;

	public: // lifecycle
		CLASS_CTOR			Settings(				const uint32_t		NUM_QUEUERS = 0,
													const uint32_t		NUM_QUEUES	= 0)
			: numQueuers(NUM_QUEUERS)
			, numQueues(NUM_QUEUES)
		{

		}

	public: // functions
		inline void			validate_QID(			const uint32_t		QID) const
		{
#ifdef _DEBUG
			if(QID >= numQueues)
				throw dpl::GeneralException(this, __LINE__, "Invalid QID");
#endif // _DEBUG
		}

		inline void			validate_SECTION_ID(	const uint32_t		SECTION_ID) const
		{
#ifdef _DEBUG
			if(SECTION_ID >= sectionsPerQ)
				throw dpl::GeneralException(this, __LINE__, "Invalid SECTION_ID");
#endif // _DEBUG
		}

		inline uint32_t		get_numSections() const
		{
			return numQueues * sectionsPerQ;
		}
	};
}