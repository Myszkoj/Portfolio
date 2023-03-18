#pragma once


#include "fqs_Queuer.h"


namespace fqs
{
	class Organizer;


	/*
		Section of the queue.
	*/
	class Section
	{
	public: // relations
		friend Organizer;

	private: // data
		std::atomic<uint32_t> firstQueuerID;
		std::atomic<uint32_t> numQueuers;

	public: // lifecycle
		CLASS_CTOR	Section()
			: firstQueuerID(Queuer::INVALID_ID)
			, numQueuers(0)
		{

		}

		CLASS_CTOR	Section(	const Section&	OTHER)
			: firstQueuerID(OTHER.firstQueuerID.load(std::memory_order_relaxed))
			, numQueuers(OTHER.numQueuers.load(std::memory_order_relaxed))
		{

		}

		CLASS_CTOR	Section(	Section&&		other) noexcept
			: firstQueuerID(other.firstQueuerID.load(std::memory_order_relaxed))
			, numQueuers(other.numQueuers.load(std::memory_order_relaxed))
		{

		}

		Section&	operator=(	const Section&	OTHER)
		{
			firstQueuerID	= OTHER.firstQueuerID.load(std::memory_order_relaxed);
			numQueuers		= OTHER.numQueuers.load(std::memory_order_relaxed);
			return *this;
		}

		Section&	operator=(	Section&&		other) noexcept
		{
			firstQueuerID	= other.firstQueuerID.load(std::memory_order_relaxed);
			numQueuers		= other.numQueuers.load(std::memory_order_relaxed);
			return *this;
		}

	private: // functions
		inline void clear()
		{
			firstQueuerID.store(Queuer::INVALID_ID, std::memory_order_relaxed);
			numQueuers.store(0, std::memory_order_relaxed);
		}
	};
}