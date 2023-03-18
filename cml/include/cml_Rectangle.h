#pragma once

#include <dpl_ReadOnly.h>
#include <dpl_GeneralException.h>
#include "cml_utilities.h"
#include "cml_TriangleMesh.h"


namespace cml
{
	class	Rectangle
	{
	public: // data
		dpl::ReadOnly<float, Rectangle> halfWidth;  // Extends on the X axis.
		dpl::ReadOnly<float, Rectangle> halfHeight;	// Extends on the Y axis.
	
	public: // functions
		CLASS_CTOR		Rectangle()
			: halfWidth(0.f)
			, halfHeight(0.f)
		{

		}

		CLASS_CTOR		Rectangle(			const float			WIDTH, 
											const float			HEIGHT)
			: halfWidth(WIDTH/2.f)
			, halfHeight(HEIGHT/2.f)
		{
			validate_width(WIDTH);
			validate_height(HEIGHT);
		}

		CLASS_CTOR		Rectangle(			const Vec2&			SIZE)
			: Rectangle(SIZE.x, SIZE.y)
		{

		}

		inline bool		operator==(			const Rectangle&	other) const
		{
			return this->halfWidth() == other.halfWidth()
				&& this->halfHeight() == other.halfHeight();
		}

		inline bool		operator!=(			const Rectangle&	other) const
		{
			return this->halfWidth() != other.halfWidth()
				|| this->halfHeight() != other.halfHeight();
		}

		inline void		set_width(			const float			newWidth)
		{
			validate_width(newWidth);
			this->halfWidth = newWidth/2.f;
		}

		inline void		set_height(			const float			newHeight)
		{
			validate_height(newHeight);
			this->halfHeight = newHeight/2.f;
		}

		inline void		set_size(			const float			width, 
											const float			height)
		{
			set_width(width);
			set_height(height);
		}

		inline void		set_size(			const Vec2&			newSize)
		{
			set_size(newSize.x, newSize.y);
		}

		inline void		set_extents(		const float			halfWidth, 
											const float			halfHeight)
		{
			validate_width(halfWidth);
			validate_height(halfHeight);

			this->halfWidth		= halfWidth;
			this->halfHeight	= halfHeight;
		}

		inline void		set_extents(		const Vec2&			extents)
		{
			set_extents(extents.x, extents.y);
		}

		inline void		scale(				const float			factor)
		{
			*halfWidth	*= factor;
			*halfHeight *= factor;
		}

		inline void		scale(				const float			sw, 
											const float			sh)
		{
			*halfWidth	*= sw;
			*halfHeight *= sh;
		}

		inline void		scale(				const Vec2&			factors)
		{
			scale(factors.x, factors.y);
		}

		inline float	width() const
		{
			return 2.f * halfWidth();
		}

		inline float	height() const
		{
			return 2.f * halfHeight();
		}

		inline Vec2		size() const
		{
			return Vec2(width(), height());
		}

		inline float	range() const
		{
			return sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
		}

		inline float	surface_area() const
		{
			return 4.f * halfWidth() * halfHeight();
		}

		TriangleMesh	to_TriangleMesh(	const uint32_t		W_SUBDIVISIONS	= 0,
											const uint32_t		H_SUBDIVISIONS	= 0) const;

	protected: // functions
		inline void		validate_width(		const float			WIDTH) const
		{
#ifdef _DEBUG
			if (WIDTH < 0.f)
				throw dpl::GeneralException(this, __LINE__, "Width cannot be negative.");
#endif // _DEBUG
		}

		inline void		validate_height(	const float			HEIGHT) const
		{
#ifdef _DEBUG
			if (HEIGHT < 0.f)
				throw dpl::GeneralException(this, __LINE__, "Height cannot be negative.");
#endif // _DEBUG
		}
	};
}