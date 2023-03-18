#include "../include/cml_Ray.h"
#include "../include/cml_AABB.h"
#include "../include/cml_OBB.h"
#include "../include/cml_Sphere.h"
#include "../include/cml_Cone.h"
#include "../include/cml_Plane.h"
#include "../include/cml_ConvexHull.h"
#include <dpl_GeneralException.h>
#include <string>


namespace cml
{
	void						Plane::LeastSquaresFitting::reset()
	{
		m_sum	= glm::vec3(0.f, 0.f, 0.f);
		m_sum2	= glm::vec2(0.f, 0.f);	
		m_xy	= 0.f;		
		m_yz	= 0.f;
		m_zx	= 0.f;
		m_count	= 0;
	}

	void						Plane::LeastSquaresFitting::add_point(	const glm::vec3& POINT)
	{
		m_sum		+= POINT;
		m_sum2.x	+= POINT.x * POINT.x;
		m_sum2.y	+= POINT.y * POINT.y;
		m_xy		+= POINT.x * POINT.y;						
		m_yz		+= POINT.y * POINT.z;
		m_zx		+= POINT.z * POINT.x;
		++m_count;
	}

	std::optional<Plane>		Plane::LeastSquaresFitting::build_plane() const
	{
		// Source: https://programmerall.com/article/3373117229/
		const float DENOM	= calculate_denom();
		if(abs(DENOM) < glm::epsilon<float>())
			return std::nullopt;

		// Calculate non-unit normal of the plane.
		const glm::vec3 V = glm::vec3(	calculate_nomP() / DENOM, 
										calculate_nomQ() / DENOM, 
										-1.f);

		const float R = calculate_nomR() / DENOM;
		const float L = glm::length(V);
		const float D = R / L;

		return Plane(V/L, -D);
	}

//=====> Plane public: // functions
	void						Plane::set_average(				const Plane*			OTHER_PLANES,
																const uint32_t			NUM_PLANES)
	{
		if(OTHER_PLANES && NUM_PLANES > 0)
		{
			float		distanceSum = 0.f;
			glm::vec3	normalSum	= CoordinateSystem::GLOBAL_ORIGIN;

			for(uint32_t index = 0; index < NUM_PLANES; ++index)
			{
				const Plane& OTHER = OTHER_PLANES[index];

				distanceSum += OTHER.distance();
				normalSum	+= OTHER.normal();
			}

			const float LENGTH = glm::length(normalSum);
			*normal		= (LENGTH > 0.f) ? normalSum / LENGTH : CoordinateSystem::GLOBAL_UP;
			*distance	= distanceSum/NUM_PLANES;
		}
	}

	std::string					Plane::get_info() const
	{
		return "Plane: N=["		+ std::to_string(normal().x) 
						+ ", "	+ std::to_string(normal().y) 
						+ ", "	+ std::to_string(normal().z) 
						+ "] D=" + std::to_string(distance());
	}

//=====> Plane public: // tests
	std::optional<glm::vec3>	Plane::get_intersection(		const glm::vec3&		RAY_ORIGIN,
																const glm::vec3&		RAY_DIRECTION) const
	{
		// source: https://stackoverflow.com/questions/23975555/how-to-do-ray-plane-intersection
		// cos angle between plane normal and the ray
		const float DENOM = calculate_dot(RAY_DIRECTION, normal);
		if(DENOM != 0.f)
		{
			const float SIGNED_DISTANCE = (distance - calculate_dot(normal, RAY_ORIGIN)) / DENOM;
			return glm::vec3(RAY_ORIGIN + SIGNED_DISTANCE * RAY_DIRECTION);
		}

		return std::nullopt;
	}

	bool						Plane::intersects(				const Ray&				RAY) const
	{
		return RAY.calculate_distance(*this).has_value();
	}

	bool						Plane::intersects(				const glm::vec3&		point) const
	{
		return abs(calculate_dot(point, normal()) - distance()) < PLUS_EPSILON;
	}

	bool						Plane::intersects(				const AABB&				aabb) const
	{
		/*
		Vec3 pMin = normal() * aabb.min();
		Vec3 pMax = normal() * aabb.max();

		if (pMin.x + pMax.y + pMax.z - distance < 0.f) return true;
		if (pMax.x + pMax.y + pMax.z - distance < 0.f) return true;
		if (pMin.x + pMax.y + pMin.z - distance < 0.f) return true;
		if (pMax.x + pMax.y + pMin.z - distance < 0.f) return true;

		if (pMin.x + pMin.y + pMax.z - distance < 0.f) return true;
		if (pMax.x + pMin.y + pMax.z - distance < 0.f) return true;
		if (pMin.x + pMin.y + pMin.z - distance < 0.f) return true;
		if (pMax.x + pMin.y + pMin.z - distance < 0.f) return true;

		return false;

		*/

		return aabb.intersects(*this);
	}

	bool						Plane::intersects(				const OBB&				obb) const
	{
		return obb.intersects(*this);
	}

	bool						Plane::intersects(				const Sphere&			sphere) const
	{
		return abs(calculate_dot(sphere.center(), normal()) - distance()) < sphere.radius();
	}

	bool						Plane::intersects(				const Cone&				cone) const
	{
		throw dpl::GeneralException(this, __LINE__, "Plane-Cone test is not implemented yet.");
		return false;
	}

	bool						Plane::intersects(				const Plane&			other) const
	{
		// Planes will not collide if they are parallel and distance between them is 0.
		if(calculate_det(this->normal(), other.normal()) == 0.f)
			return this->distance() == other.distance(); // Note that we check distance to origin of the coordinate system.

		return true;
	}

	bool						Plane::intersects(				const ConvexHull&		convexHull) const
	{
		throw dpl::GeneralException(this, __LINE__, "Plane-ConvexHull test is not implemented yet.");
		return false;
	}
}