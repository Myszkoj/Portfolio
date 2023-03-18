#pragma once


#include "cml_Rectangle.h"


namespace cml
{
	class	AABR : public Rectangle
	{
	public: // data
		dpl::ReadOnly<Vec2, AABR> center;

	public: // functions
		CLASS_CTOR			AABR()
			: center(0.f, 0.f)
		{
			
		}

		CLASS_CTOR			AABR(				const Vec2&			min, 
												const Vec2&			max);

		CLASS_CTOR			AABR(				const Rectangle&	rectangle, 
												const Vec2&			center);

		CLASS_CTOR			AABR(				const Vec2*			points,
												const uint32_t		numPoints);

		inline bool			operator==(			const AABR&			other) const
		{
			return Rectangle::operator==(other) && (this->center() == other.center());
		}

		inline bool			operator!=(			const AABR&			other) const
		{
			return Rectangle::operator!=(other) || (this->center() != other.center());
		}

		void				reset(				const Vec2&			min, 
												const Vec2&			max);

		void				reset(				const Vec2*			points,
												const uint32_t		numPoints);

		inline void			set_center(			const Vec2&			newCenter)
		{
			this->center = newCenter;
		}

		inline float		min_x() const
		{
			return center().x - halfWidth;
		}

		inline float		min_y() const
		{
			return center().y - halfHeight;
		}

		inline float		max_x() const
		{
			return center().x + halfWidth;
		}

		inline float		max_y() const
		{
			return center().y + halfHeight;
		}

		inline Vec2			min() const
		{
			return Vec2(min_x(), min_y());
		}

		inline Vec2			max() const
		{
			return Vec2(max_x(), max_y());
		}

		Vec2				closest_point(		const Vec2&			point) const;

		inline float		project_size(		const Vec2&			axis) const
		{
			return abs(axis.x * halfWidth())
				 + abs(axis.y * halfHeight());
		}

		inline void			move(				const Vec2&			offset)
		{
			*center += offset;
		}

		void				extend(				const Vec2&			point);

		void				extend(				const Vec2&			min, 
												const Vec2&			max);

		inline void			extend(				const AABR&			other)
		{
			extend(other.min(), other.max());
		}

		float				distance(			const Vec2&			point) const;

		bool				contains(			const Vec2&			point) const;

		bool				contains(			const AABR&			other) const;

		bool				intersects(			const Vec2&			point) const;

		bool				intersects(			const AABR&			other) const;
	};
}