#pragma once

#include "fqs_utilities.h"

#pragma warning( disable : 26812 ) // unscoped enum

namespace fqs
{
	class Organizer;


	/*
		Person or spot in a queue.
	*/
	class Queuer
	{
	public: // relations
		friend Organizer;

	public: // constants (OBSOLETE?)
		static const uint32_t TYPE_BITS			= 1;
		static const uint32_t INDEX_BITS		= 31;
		static const uint32_t INVALID_NUMBER	= 0x7FFFFFFF;
		static const uint32_t INVALID_ID		= 0xFFFFFFFF;
		static const uint32_t ACCURACY_CM		= 100;

	private: // data
		uint32_t	m_uniqueID;
		uint32_t	m_QID;
		uint32_t	m_nextID; // ID of the queuer behind this one.
		uint32_t	m_dist;
		float		m_value; // Cumulative value

	public: // lifecycle
		CLASS_CTOR		Queuer()
			: m_uniqueID(0)
			, m_QID(INVALID_ID)
			, m_nextID(INVALID_ID)
			, m_dist(INVALID_NUMBER)
			, m_value(0.f)
		{
			
		}

	public: // functions
		inline void		update_with_cm(	const uint32_t	UNIQUE_ID,
										const uint32_t	NEW_QID,
										const uint32_t	DISTANCE_IN_CM,
										const float		QUEUER_VALUE = 1.f)
		{
			m_uniqueID	= UNIQUE_ID;
			m_QID		= NEW_QID;
			m_dist		= DISTANCE_IN_CM;
			m_value		= 0;
		}

		inline void		update_with_m(	const uint32_t	UNIQUE_ID,
										const uint32_t	NEW_QID,
										const float		DISTANCE_IN_METERS,
										const float		QUEUER_VALUE = 1.f)
		{
			update_with_cm(UNIQUE_ID, NEW_QID, (uint32_t)(ACCURACY_CM * DISTANCE_IN_METERS), QUEUER_VALUE);
		}

		inline uint32_t get_QID() const
		{
			return m_QID;
		}

		inline float	get_value() const
		{
			return m_value;
		}

		inline uint32_t	get_distance_cm() const
		{
			return m_dist;
		}

		inline float	get_distance_m() const
		{
			return (float)m_dist / 100.f;
		}

		inline void		invalidate()
		{
			m_QID	= INVALID_ID;
			//m_dist = std::numeric_limits<float>::quiet_NaN();
		}

		inline bool		is_valid(		const uint32_t	NUM_QUEUES) const
		{
			return m_QID < NUM_QUEUES;
		}
	};
}