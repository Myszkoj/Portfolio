#pragma once


#include "cml_TriangleMesh.h"


namespace cml
{
	class Ray;
	class AABB;
	class OBB;
	class Sphere;
	class Plane;
	class ConvexHull;



	/*
		https://stackoverflow.com/questions/22023977/detect-if-a-cube-and-a-cone-intersect-each-other
		https://www.geometrictools.com/Documentation/IntersectionSphereCone.pdf
		https://gist.github.com/jcayzac/1241829
	*/
	class	Cone
	{
	public: // data
		dpl::ReadOnly<Vec3,		Cone> apex;
		dpl::ReadOnly<Vec3,		Cone> axis; // Points towards cone base.
		dpl::ReadOnly<float,	Cone> height;
		dpl::ReadOnly<float,	Cone> fi;
		dpl::ReadOnly<float,	Cone> sinFi;
		dpl::ReadOnly<float,	Cone> cosFi;

	public: // lifecycle
		CLASS_CTOR			Cone()
			: apex(0.f, 0.f, 0.f)
			, axis(0.f, 0.f, 0.f)
			, height(0.f)
			, fi(0.f)
			, sinFi(0.f)
			, cosFi(0.f)
		{

		}

		CLASS_CTOR			Cone(				const Vec3&			apex,
												const Vec3&			axis,
												const float			height,
												const float			fiRadians)
			: apex(apex)
			, axis(axis)
			, height(glm::max(0.f, height))
			, fi(0.f)
			, sinFi(0.f)
			, cosFi(0.f)
		{
			set_fiAngle(fiRadians);
		}

	public: // functions
		inline void			set_apex(			const Vec3&			newApex)
		{
			this->apex = newApex;
		}

		inline void			set_axis(			const Vec3&			newAxis)
		{
			this->axis = newAxis;
		}

		inline void			set_height(			const float			newHeight)
		{
			this->height = glm::max(0.f, newHeight);
		}

		inline void			set_fiAngle(		const float			radians)
		{
			if(this->fi() != radians)
			{
				this->fi	= glm::clamp(radians, 0.f, ANGLE_90);
				this->sinFi = glm::sin(fi());
				this->cosFi = glm::cos(fi());
			}
		}

		inline float		calculate_radius() const
		{
			return this->height() * sinFi() / cosFi();
		}

		inline float		calculate_volume() const
		{
			const float radius = calculate_radius();
			return glm::pi<float>()*radius*(radius + glm::sqrt(height*height + radius*radius));
		}

		TriangleMesh		to_TriangleMesh(	const uint32_t		SLICES) const;

	public: //  collision tests
		bool				above(				const Plane&		plane) const;

		bool				below(				const Plane&		plane) const;

		bool				intersects(			const Ray&			ray) const;

		bool				intersects(			const Vec3&			point) const;

		bool				intersects(			const AABB&			aabb) const;

		bool				intersects(			const OBB&			obb) const;

		bool				intersects(			const Sphere&		sphere) const;

		bool				intersects(			const Cone&			other) const;

		bool				intersects(			const Plane&		plane) const;

		bool				intersects(			const ConvexHull&	convexHull) const;
	};
}