#pragma once


#include "cml_Cuboid.h"
#include "cml_CoordinateSystem.h"


namespace cml
{
	class Ray;
	class AABB;
	class OBB;
	class Sphere;
	class Cone;
	class Plane;
	class ConvexHull;



	/*
		https://github.com/juj/MathGeoLib/blob/master/src/Geometry/OBB.cpp
	*/
	class OBB	: public Cuboid
				, public CoordinateSystem
	{
	public: // types
		using Instances = std::vector<OBB>;

	private: // invalid functions
		using CoordinateSystem::transform;

	public: // lifecycle
		CLASS_CTOR		OBB();

		CLASS_CTOR		OBB(				const AABB&				aabb);
	
		CLASS_CTOR		OBB(				const AABB&				aabb,
											const Mat4&				TRANSFORMATION);

	public: // functions
		void			reset(				const AABB&				aabb);
			
		void			reset(				const AABB&				aabb,
											const Mat4&				TRANSFORMATION);

		void			transform(			const Mat4&				TRANSFORMATION);

		/*
			Calculate axis aligned cuboid that encapsulates this obb.
		*/
		Cuboid			align_size() const;

		Vec3			corner(				Corner					cornerID) const;

		Vec3			face_center(		Face					faceID) const;

		Vec3			face_normal(		Face					faceID) const;

		Plane			face_plane(			Face					faceID) const;

		Vec3			closest_point(		const Vec3&				point) const;

		inline float	project_size(		const Vec3&				axis) const
		{
			return abs(calculate_dot(axis, front()) * halfWidth())
				 + abs(calculate_dot(axis, up()) * halfHeight())
				 + abs(calculate_dot(axis, right()) * halfDepth());
		}

	public: // intersection
		bool			intersects(			const Vec3&				point) const;

		bool			intersects(			const AABB&				aabb) const;

		bool			intersects(			const OBB&				OTHER) const;

		bool			intersects(			const Sphere&			sphere) const;

		bool			intersects(			const Cone&				cone) const;

		bool			intersects(			const Plane&			plane) const;

		bool			intersects(			const ConvexHull&		convexHull) const;

	public: // other
		void			extend(				const Vec3&				POINT);

		void			extend(				const OBB&				OTHER);

	private: // tests
		bool			is_separation_axis(	const OBB&				OTHER,
											const Vec3&				TO_OTHER,
											const Vec3&				AXIS) const;
	};
}