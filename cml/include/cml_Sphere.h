#pragma once


#include "cml_TriangleMesh.h"


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
		https://www.geometrictools.com/Documentation/DynamicCollisionDetection.pdf
	*/
	class	Sphere
	{
	public: // data
		dpl::ReadOnly<Vec3,		Sphere> center;
		dpl::ReadOnly<float,	Sphere> radius;

	public: // lifecycle
		CLASS_CTOR				Sphere(				float						radius = 0.f)
			: center(0.f, 0.f, 0.f)
			, radius(radius)
		{
		}

		CLASS_CTOR				Sphere(				const Vec3&					center, 
													float						radius)
			: center(center)
			, radius(radius)
		{
		}

		CLASS_CTOR				Sphere(				const Sphere&				other, 
													const Vec3&					offset)
			: center(other.center() + offset)
			, radius(other.radius)
		{
		}

		CLASS_CTOR				Sphere(				const Sphere&				other, 
													float						dR)
			: center(other.center())
			, radius(glm::max(0.f, other.radius + dR))
		{
		}

		CLASS_CTOR				Sphere(				const AABB&					aabb);

		inline Sphere			operator+(			const Vec3&					offset) const
		{
			return Sphere(*this, offset); 
		}

		inline Sphere			operator-(			const Vec3&					offset) const
		{
			return Sphere(*this, -offset); 
		}

		inline Sphere			operator+(			float						dR) const
		{
			return Sphere(*this, dR);
		}

		inline Sphere			operator-(			float						dR) const
		{
			return Sphere(*this, dR);
		}

	public: // functions
		inline void				set_center(			const Vec3&					position)
		{
			this->center = position;
		}

		inline void				set_radius(			float						radius)
		{
			this->radius = radius;
		}

		inline float			calculate_volume() const
		{
			return (4.f * glm::pi<float>() * radius() * radius() * radius()) / 3.f;
		}

		TriangleMesh			to_TriangleMesh(	const uint32_t				MERIDIANS, 
													const uint32_t				PARALLELS) const;

	public: // collision tests
		inline bool				contains(			const Vec3&					point) const
		{
			return calculate_distance(center(), point) < radius();
		}

		inline bool				contains(			const Sphere&				other) const
		{
			return calculate_distance(center(), other.center()) + other.radius() < radius();
		}

		bool					contains(			const AABB&					aabb) const;

		bool					contains(			const OBB&					obb) const;

		bool					above(				const Plane&				plane) const;

		bool					below(				const Plane&				plane) const;

		bool					intersects(			const Ray&					ray) const;

		bool					intersects(			const Vec3&					point) const;

		bool					intersects(			const AABB&					aabb) const;

		bool					intersects(			const OBB&					obb) const;

		bool					intersects(			const Sphere&				other) const;

		bool					intersects(			const Cone&					cone) const;

		bool					intersects(			const Plane&				plane) const;

		bool					intersects(			const ConvexHull&			convexHull) const;
	};
}