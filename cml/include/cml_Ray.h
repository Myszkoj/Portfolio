#pragma once


#include <optional>
#include <dpl_ReadOnly.h>
#include "cml_CoordinateSystem.h"


namespace cml
{
	class Plane;


	class Ray
	{
	public: // data
		dpl::ReadOnly<Vec3, Ray> origin;
		dpl::ReadOnly<Vec3, Ray> direction;

	public: // lifecycle
		CLASS_CTOR					Ray()
			: origin(CoordinateSystem::GLOBAL_ORIGIN)
			, direction(CoordinateSystem::GLOBAL_FRONT)
		{

		}

		CLASS_CTOR					Ray(				const Vec3&		ORIGIN,
														const Vec3&		DIRECTION)
			: origin(ORIGIN)
			, direction(DIRECTION)
		{

		}

		CLASS_CTOR					Ray(				const Vec2&		TARGET,
														const Vec2&		VIEW_MIN,
														const Vec2&		VIEW_MAX,
														const Mat4&		VIEW_PROJECTION)
			: origin(CoordinateSystem::GLOBAL_ORIGIN)
			, direction(CoordinateSystem::GLOBAL_FRONT)
		{
			set(TARGET, VIEW_MIN, VIEW_MAX, VIEW_PROJECTION);
		}

	public: // functions
		inline void					set_origin(			const float		X,
														const float		Y,
														const float		Z)
		{
			origin->x = X;
			origin->y = Y;
			origin->z = Z;
		}

		inline void					set_origin(			const Vec3&		NEW_ORIGIN)
		{
			this->origin = NEW_ORIGIN;
		}

		void						set(				const Vec2&		TARGET,
														const Vec2&		VIEW_MIN,
														const Vec2&		VIEW_MAX,
														const Mat4&		VIEW_PROJECTION);

		inline Vec3					calculate_ahead(	const float		DISTANCE) const
		{
			return origin() + direction() * DISTANCE;
		}

		/*
			Returns positive value when plane and ray are facing towards each other, 
			negative when away from each other and nullopt if ray is perpendicular to the plane normal.
		*/
		std::optional<float>		calculate_distance(	const Plane&	PLANE) const;

		inline std::optional<Vec3>	test_intersection(	const Plane&	PLANE) const
		{
			if(const auto DISTANCE = calculate_distance(PLANE))
				return calculate_ahead(DISTANCE.value());

			return std::nullopt;
		}
	};
}