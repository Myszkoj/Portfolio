#pragma once


#include "upf_Box.h"


namespace upf
{
	class Series
	{
	private: // data
		void*			m_objects;
		uint32_t		m_size;
		uint32_t		m_stride;	// Size of the User object.
		uint32_t		m_offset;	// Byte offset to the Box base.
		cml::HVSize		m_boxSize;

	public: // lifecycle
		CLASS_CTOR			Series()
			: m_objects(nullptr)
			, m_size(0)
			, m_stride(0)
			, m_offset(0)
			, m_boxSize(0.f, 0.f)
		{
			
		}

		template<typename T>
		CLASS_CTOR			Series(		T*					objects,
										const uint32_t		NUM_OBJECTS,
										const cml::HVSize	BOX_SIZE)
			: m_objects(objects)
			, m_size(NUM_OBJECTS)
			, m_stride((uint32_t)sizeof(T))
			, m_offset((uint32_t)((uint64_t)static_cast<const Box*>((const T*)(nullptr))))
			, m_boxSize(BOX_SIZE)
			
		{
			static_assert(std::is_base_of<Box, T>::value, "T must be publicly derived from Box.");
		}

		CLASS_CTOR			Series(		const Series&		OTHER)
			: m_objects(OTHER.m_objects)
			, m_size(OTHER.m_size)
			, m_stride(OTHER.m_stride)
			, m_offset(OTHER.m_offset)
			, m_boxSize(OTHER.m_boxSize)
		{

		}

		CLASS_CTOR			Series(		Series&&			other) noexcept
			: m_objects(other.m_objects)
			, m_size(other.m_size)
			, m_stride(other.m_stride)
			, m_offset(other.m_offset)
			, m_boxSize(other.m_boxSize)
		{

		}

		Series&				operator=(	const Series&		OTHER)
		{
			m_objects	= OTHER.m_objects;
			m_size		= OTHER.m_size;
			m_stride	= OTHER.m_stride;
			m_offset	= OTHER.m_offset;
			m_boxSize	= OTHER.m_boxSize;
			return *this;
		}

		Series&				operator=(	Series&&			other) noexcept
		{
			m_objects	= other.m_objects;
			m_size		= other.m_size;
			m_stride	= other.m_stride;
			m_offset	= other.m_offset;
			m_boxSize	= other.m_boxSize;
			return *this;
		}

	public: // functions
		inline char*		objects() const
		{
			return (char*)m_objects;
		}

		inline uint32_t		size() const
		{
			return m_size;
		}

		inline uint32_t		stride() const
		{
			return m_stride;
		}

		inline uint32_t		offset() const
		{
			return m_offset;
		}

		inline cml::HVSize	boxSize() const
		{
			return m_boxSize;
		}
	};
}