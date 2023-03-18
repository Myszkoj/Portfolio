#include "..//include/cml_AABR.h"


namespace cml
{
	CLASS_CTOR	AABR::AABR(					const Vec2&			min, 
											const Vec2&			max) 
		: Rectangle(max-min)
		, center((min+max)/2.f)
	{
	}

	CLASS_CTOR	AABR::AABR(					const Rectangle&	rectangle, 
											const Vec2&			center) 
		: Rectangle(rectangle)
		, center(center)
	{
	}

	CLASS_CTOR	AABR::AABR(					const Vec2*			points,
											const uint32_t		numPoints)
		: Rectangle(0.f, 0.f)
		, center(0.f, 0.f)
	{
		reset(points, numPoints);
	}

	void		AABR::reset(				const Vec2*			points,
											const uint32_t		numPoints)
	{
		if(points && numPoints > 0)
		{
			Vec2 min = points[0];
			Vec2 max = points[0];

			for (uint32_t n = 1; n < numPoints; ++n)
			{
				auto& current = points[n];

				if(current.x < min.x)
					min.x = current.x;

				if(current.y < min.y)
					min.y = current.y;

				if(current.x > max.x)
					max.x = current.x;

				if(current.y > max.y)
					max.y = current.y;
			}

			center = (max + min) / 2.f;
			set_size(max - min);
		}
		else
		{
			Rectangle::set_extents(0.f, 0.f);
			center->x = 0.f;
			center->y = 0.f;
		}
	}

	void		AABR::reset(				const Vec2&			min, 
											const Vec2&			max)
	{
		Rectangle::set_size(max-min);
		center = (min+max)/2.f;
	}

	Vec2		AABR::closest_point(		const Vec2&			point) const
	{
		const Vec2 min = this->min();
		const Vec2 max = this->max();

		return Vec2(glm::clamp(point.x, min.x, max.x),
					glm::clamp(point.y, min.y, max.y));
	}

	void		AABR::extend(				const Vec2&			point)
	{
		const Vec2 thisMin = this->min();
		const Vec2 thisMax = this->max();

		Vec2 newMin(glm::min(thisMin.x, point.x),
					glm::min(thisMin.y, point.y));

		Vec2 newMax(glm::max(thisMax.x, point.x),
					glm::max(thisMax.y, point.y));

		reset(newMin, newMax);
	}

	void		AABR::extend(				const Vec2&			min, 
											const Vec2&			max)
	{
		const Vec2 thisMin = this->min();
		const Vec2 thisMax = this->max();

		Vec2 newMin(glm::min(thisMin.x, min.x),
					glm::min(thisMin.y, min.y));

		Vec2 newMax(glm::max(thisMax.x, max.x),
					glm::max(thisMax.y, max.y));

		reset(newMin, newMax);
	}

	float		AABR::distance(				const Vec2&			point) const
	{
		float dx = abs(center().x - point.x) - halfWidth;
		float dy = abs(center().y - point.y) - halfHeight;

		if(dx > 0.f)
		{
			if(dy > 0.f)
			{
				return sqrt(dx*dx + dy*dy);
			}
			else
			{
				return dx;
			}
		}
		else
		{
			if(dy > 0.f)
			{
				return dy;
			}
		}

		return 0.f;
	}

	bool		AABR::contains(				const Vec2&			point) const
	{
		if(abs(center().x - point.x) > halfWidth)
			return false;

		if(abs(center().y - point.y) > halfHeight)
			return false;

		return true;
	}

	bool		AABR::contains(				const AABR&			other) const
	{
		if(abs(center().x - other.center().x) + other.halfWidth > halfWidth)
			return false;

		if(abs(center().y - other.center().y) + other.halfHeight > halfHeight)
			return false;

		return true;
	}

	bool		AABR::intersects(			const Vec2&			point) const
	{
		if (abs(center().x - point.x) > halfWidth)
			return false;

		if (abs(center().y - point.y) > halfHeight)
			return false;

		return true;
	}

	bool		AABR::intersects(			const AABR&			other) const
	{
		if(abs(center().x - other.center().x) > halfWidth + other.halfWidth)
			return false;

		if(abs(center().y - other.center().y) > halfHeight + other.halfHeight)
			return false;

		return true;
	}
}