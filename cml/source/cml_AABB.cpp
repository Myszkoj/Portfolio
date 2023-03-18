#include "../include/cml_Ray.h"
#include "../include/cml_AABB.h"
#include "../include/cml_OBB.h"
#include "../include/cml_Sphere.h"
#include "../include/cml_Cone.h"
#include "../include/cml_Plane.h"
#include "../include/cml_ConvexHull.h"

#pragma warning( disable : 26812)

namespace cml
{
	template<typename IndexT>
	void		encapsulate_points(			const Vec3*			POINTS,
											const IndexT*		INDICES,
											const uint32_t		NUM_INDICES,
											AABB&				output)
	{
		if(POINTS && INDICES && NUM_INDICES > 0)
		{
			auto& first = INDICES[0];

			Vec3 min = POINTS[first];
			Vec3 max = POINTS[first];

			for (uint32_t i = 1; i < NUM_INDICES; ++i)
			{
				auto& index		= INDICES[i];
				auto& current	= POINTS[index];

				if(current.x < min.x)
					min.x = current.x;

				if(current.y < min.y)
					min.y = current.y;

				if(current.z < min.z)
					min.z = current.z;

				if(current.x > max.x)
					max.x = current.x;

				if(current.y > max.y)
					max.y = current.y;

				if(current.z > max.z)
					max.z = current.z;
			}

			output.set_center((max + min) / 2.f);
			output.set_size(max - min);
		}
		else
		{
			output.set_extents(0.f, 0.f, 0.f);
			output.set_center(0.f, 0.f, 0.f);
		}
	}


//=====> AABB -> public lifecycle
	CLASS_CTOR	AABB::AABB(					const Vec3&			min, 
											const Vec3&			max) 
		: Cuboid(max-min)
		, center((min+max)/2.f)
	{
	}

	CLASS_CTOR	AABB::AABB(					const Cuboid&		cuboid, 
											const Vec3&			center) 
		: Cuboid(cuboid)
		, center(center)
	{
	}

	CLASS_CTOR	AABB::AABB(					const OBB&			obb)
		: Cuboid(obb.align_size())
		, center(obb.origin())
	{
		
	}

	CLASS_CTOR	AABB::AABB(					const Sphere&		sphere)
		: Cuboid(sphere.radius(), sphere.radius(), sphere.radius())
		, center(sphere.center())
	{

	}

	CLASS_CTOR	AABB::AABB(					const Vec3*			POINTS,
											const uint32_t		NUM_POINTS)
		: AABB()
	{
		reset(POINTS, NUM_POINTS);
	}

	CLASS_CTOR	AABB::AABB(					const Vec3*			POINTS,
											const uint8_t*		INDICES,
											const uint32_t		NUM_INDICES)
		: AABB()
	{
		encapsulate_points(POINTS, INDICES, NUM_INDICES, *this);
	}

	CLASS_CTOR	AABB::AABB(					const Vec3*			POINTS,
											const uint16_t*		INDICES,
											const uint32_t		NUM_INDICES)
		: AABB()
	{
		encapsulate_points(POINTS, INDICES, NUM_INDICES, *this);
	}

	CLASS_CTOR	AABB::AABB(					const Vec3*			POINTS,
											const uint32_t*		INDICES,
											const uint32_t		NUM_INDICES)
		: AABB()
	{
		encapsulate_points(POINTS, INDICES, NUM_INDICES, *this);
	}

//=====> AABB -> public functions
	OBB			AABB::operator*(			const Mat4&			TRANSFORMATION) const
	{
		return OBB(*this, TRANSFORMATION);
	}

	void		AABB::reset(				const Vec3*			POINTS,
											const uint32_t		NUM_POINTS)
	{
		if(POINTS && NUM_POINTS > 0)
		{
			Vec3 min = POINTS[0];
			Vec3 max = POINTS[0];

			for (uint32_t n = 1; n < NUM_POINTS; ++n)
			{
				auto& current = POINTS[n];

				if(current.x < min.x)
					min.x = current.x;

				if(current.y < min.y)
					min.y = current.y;

				if(current.z < min.z)
					min.z = current.z;

				if(current.x > max.x)
					max.x = current.x;

				if(current.y > max.y)
					max.y = current.y;

				if(current.z > max.z)
					max.z = current.z;
			}

			center = (max + min) / 2.f;
			set_size(max - min);
		}
		else
		{
			Cuboid::set_extents(0.f, 0.f, 0.f);
			center->x = 0.f;
			center->y = 0.f;
			center->z = 0.f;
		}
	}

	void		AABB::reset(				const Vec3*			POINTS,
											const uint8_t*		INDICES,
											const uint32_t		NUM_INDICES)
	{
		encapsulate_points(POINTS, INDICES, NUM_INDICES, *this);
	}

	void		AABB::reset(				const Vec3*			POINTS,
											const uint16_t*		INDICES,
											const uint32_t		NUM_INDICES)
	{
		encapsulate_points(POINTS, INDICES, NUM_INDICES, *this);
	}

	void		AABB::reset(				const Vec3*			POINTS,
											const uint32_t*		INDICES,
											const uint32_t		NUM_INDICES)
	{
		encapsulate_points(POINTS, INDICES, NUM_INDICES, *this);
	}

	void		AABB::reset(				const Vec3&			min, 
											const Vec3&			max)
	{
		Cuboid::set_size(max-min);
		center = (min+max)/2.f;
	}

	Vec3		AABB::corner(				const Corner		CORNER_ID) const
	{
		switch(CORNER_ID)
		{
		default: // Should anything go wrong, we return the first case.
		case eLEFT_BOTTOM_BACK:		return Vec3(center().x	-halfWidth(),		center().y	-halfHeight(),	center().z	-halfDepth());
		case eLEFT_BOTTOM_FRONT:	return Vec3(center().x	-halfWidth(),		center().y	-halfHeight(),	center().z	+halfDepth());
		case eLEFT_TOP_BACK:		return Vec3(center().x	-halfWidth(),		center().y	+halfHeight(),	center().z	-halfDepth());
		case eLEFT_TOP_FRONT:		return Vec3(center().x	-halfWidth(),		center().y	+halfHeight(),	center().z	+halfDepth());
		case eRIGHT_BOTTOM_BACK:	return Vec3(center().x	+halfWidth(),		center().y	-halfHeight(),	center().z	-halfDepth());
		case eRIGHT_BOTTOM_FRONT:	return Vec3(center().x	+halfWidth(),		center().y	-halfHeight(),	center().z	+halfDepth());
		case eRIGHT_TOP_BACK:		return Vec3(center().x	+halfWidth(),		center().y	+halfHeight(),	center().z	-halfDepth());
		case eRIGHT_TOP_FRONT:		return Vec3(center().x	+halfWidth(),		center().y	+halfHeight(),	center().z	+halfDepth());
		}
	}

	Vec3		AABB::face_center(			const Face			FACE_ID) const
	{
		switch(FACE_ID)
		{
		default: // Should anything go wrong, we return the first case.
		case eLEFT:		return Vec3(center().x	-halfWidth(),	center().y,					center().z);
		case eRIGHT:	return Vec3(center().x	+halfWidth(),	center().y,					center().z);
		case eBOTTOM:	return Vec3(center().x,					center().y	-halfHeight(),	center().z);
		case eTOP:		return Vec3(center().x,					center().y	+halfHeight(),	center().z);
		case eBACK:		return Vec3(center().x,					center().y,					center().z	-halfDepth());
		case eFRONT:	return Vec3(center().x,					center().y,					center().z	+halfDepth());
		}
	}

	Vec3		AABB::face_normal(			const Face			FACE_ID) const
	{
		switch(FACE_ID)
		{
		default: // Should anything go wrong, we return the first case.
		case eLEFT:		return Vec3(-1.f,	0.f,	0.f);
		case eRIGHT:	return Vec3(1.f,	0.f,	0.f);
		case eBOTTOM:	return Vec3(0.f,	-1.f,	0.f);
		case eTOP:		return Vec3(0.f,	1.f,	0.f);
		case eBACK:		return Vec3(0.f,	0.f,	-1.f);
		case eFRONT:	return Vec3(0.f,	0.f,	1.f);
		}
	}

	Plane		AABB::face_plane(			const Face			FACE_ID) const
	{
		return Plane(face_center(FACE_ID), face_normal(FACE_ID));
	}

	Vec3		AABB::closest_point(		const Vec3&			point) const
	{
		Vec3 min = this->min();
		Vec3 max = this->max();

		return Vec3(glm::clamp(point.x, min.x, max.x),
					glm::clamp(point.y, min.y, max.y),
					glm::clamp(point.z, min.z, max.z));
	}

	void		AABB::extend(				const Vec3&			point)
	{
		const Vec3 thisMin = this->min();
		const Vec3 thisMax = this->max();

		Vec3 newMin(glm::min(thisMin.x, point.x),
					glm::min(thisMin.y, point.y),
					glm::min(thisMin.z, point.z));

		Vec3 newMax(glm::max(thisMax.x, point.x),
					glm::max(thisMax.y, point.y),
					glm::max(thisMax.z, point.z));

		reset(newMin, newMax);
	}

	void		AABB::extend(				const Vec3&			min, 
											const Vec3&			max)
	{
		const Vec3 thisMin = this->min();
		const Vec3 thisMax = this->max();

		Vec3 newMin(glm::min(thisMin.x, min.x),
					glm::min(thisMin.y, min.y),
					glm::min(thisMin.z, min.z));

		Vec3 newMax(glm::max(thisMax.x, max.x),
					glm::max(thisMax.y, max.y),
					glm::max(thisMax.z, max.z));

		reset(newMin, newMax);
	}

	float		AABB::distance(				const Vec3&			point) const
	{
		float dx = abs(center().x - point.x) - halfWidth();
		float dy = abs(center().y - point.y) - halfHeight();
		float dz = abs(center().z - point.z) - halfDepth();

		if(dx > 0.f)
		{
			if(dy > 0.f)
			{
				if(dz > 0.f)
				{
					return sqrt(dx*dx + dy*dy + dz*dz);
				}
				else
				{
					return sqrt(dx*dx + dy*dy);
				}
			}
			else
			{
				if(dz > 0.f)
				{
					return sqrt(dx*dx + dz*dz);
				}
				else
				{
					return dx;
				}
			}
		}
		else
		{
			if(dy > 0.f)
			{
				if(dz > 0.f)
				{
					return sqrt(dy*dy + dz*dz);
				}
				else
				{
					return dy;
				}
			}
			else
			{
				if(dz > 0.f)
				{
					return dz;
				}
				else
				{
					// 0
				}
			}
		}

		return 0.f;
	}

	float		AABB::distance(				const Sphere&		sphere) const
	{
		return glm::max(0.f, distance(sphere.center()) - sphere.radius());
	}

	bool		AABB::contains(				const Vec3&			point) const
	{
		if(abs(center().x - point.x) > halfWidth())
			return false;

		if(abs(center().y - point.y) > halfHeight())
			return false;

		if(abs(center().z - point.z) > halfDepth())
			return false;

		return true;
	}

	bool		AABB::contains(				const AABB&			other) const
	{
		if(abs(center().x - other.center().x) + other.halfWidth() > halfWidth())
			return false;

		if(abs(center().y - other.center().y) + other.halfHeight() > halfHeight())
			return false;

		if(abs(center().z - other.center().z) + other.halfDepth() > halfDepth())
			return false;

		return true;
	}

	bool		AABB::above(				const Plane&		plane) const
	{
		float signedDistance = calculate_dot(plane.normal, center() - plane.distance());

		if(signedDistance <= 0.f)
			return false;

		return signedDistance > project_size(plane.normal());
	}

	bool		AABB::below(				const Plane&		plane) const
	{
		float signedDistance = calculate_dot(plane.normal, center() - plane.distance());

		if(signedDistance >= 0.f)
			return false;

		return abs(signedDistance) > project_size(plane.normal());
	}

	bool		AABB::intersects(			const Ray&			ray) const
	{
		float Tmin	= 0.f;
		float Tmax	= std::numeric_limits<float>::max();

		/*	The user should have inputted values for Tmin and Tmax to specify the desired subrange 
			[Tmin, Tmax] of the line for this intersection test.

			For a Line-AABB test, pass in
				Tmin	= -FLOAT_INF;
				exitDist	= FLOAT_INF;

			For a Ray-AABB test, pass in
				Tmin	= 0.f;
				exitDist	= FLOAT_INF;

			For a LineSegment-AABB test, pass in
				Tmin	= 0.f;
				exitDist	= LineSegment.Length();
		*/

		// Signed distance on the X axis.
		float sdX = center().x - ray.origin().x;

		// Test X axis.
		if (ray.direction().x != 0.f)
		{
			float T1	= (sdX - halfWidth()) / ray.direction().x;
			float T2	= (sdX + halfWidth()) / ray.direction().x;

			if (T1 < T2)
			{
				Tmin	= glm::max(T1, Tmin); 
				Tmax	= glm::min(T2, Tmax);
			}
			else // Swap t1 and t2.
			{
				Tmin	= glm::max(T2, Tmin); 
				Tmax	= glm::min(T1, Tmax);
			}

			if (Tmin > Tmax)
				return false; // Box is missed since we "exit" before entering it.
		}
		else if (abs(sdX) > halfWidth())
			return false; // The ray can't possibly enter the box, abort.

		// Signed distance on the Y axis.
		float sdY = center().y - ray.origin().y;;

		// Test Y axis.
		if (ray.direction().y != 0.f)
		{
			float T1	= (sdY - halfHeight()) / ray.direction().y;
			float T2	= (sdY + halfHeight()) / ray.direction().y;

			if (T1 < T2)
			{
				Tmin	= glm::max(T1, Tmin); 
				Tmax	= glm::min(T2, Tmax);
			}
			else // Swap t1 and t2.
			{
				Tmin	= glm::max(T2, Tmin); 
				Tmax	= glm::min(T1, Tmax);
			}

			if (Tmin > Tmax)
				return false; // Box is missed since we "exit" before entering it.
		}
		else if (abs(sdY) > halfHeight())
			return false; // The ray can't possibly enter the box, abort.

		// Signed distance on the Z axis.
		float sdZ = center().z - ray.origin().z;

		// Test Z axis.
		if (ray.direction().z != 0.f)
		{
			float T1	= (sdZ - halfDepth()) / ray.direction().z;
			float T2	= (sdZ + halfDepth()) / ray.direction().z;

			if (T1 < T2)
			{
				Tmin	= glm::max(T1, Tmin); 
				Tmax	= glm::min(T2, Tmax);
			}
			else // Swap t1 and t2.
			{
				Tmin	= glm::max(T2, Tmin); 
				Tmax	= glm::min(T1, Tmax);
			}

			if (Tmin > Tmax)
				return false; // Box is missed since we "exit" before entering it.
		}
		else if (abs(sdZ) > halfDepth())
			return false; // The ray can't possibly enter the box, abort.

		return true;
	}

	bool		AABB::intersects(			const Vec3&			point) const
	{
		if (abs(center().x - point.x) > halfWidth())
			return false;

		if (abs(center().y - point.y) > halfHeight())
			return false;

		if (abs(center().z - point.z) > halfDepth())
			return false;

		return true;
	}

	bool		AABB::intersects(			const AABB&			other) const
	{
		if(abs(center().x - other.center().x) > halfWidth() + other.halfWidth())
			return false;

		if(abs(center().y - other.center().y) > halfHeight() + other.halfHeight())
			return false;

		if(abs(center().z - other.center().z) > halfDepth() + other.halfDepth())
			return false;

		return true;
	}

	bool		AABB::intersects(			const OBB&			obb) const
	{
		return obb.intersects(*this);
	}

	bool		AABB::intersects(			const Sphere&		sphere) const
	{
		return sphere.intersects(*this);
	}

	bool		AABB::intersects(			const Cone&			cone) const
	{
		return cone.intersects(*this);
	}

	bool		AABB::intersects(			const Plane&		plane) const
	{
		// Compute the distance of this OBB center from the plane.
		float s = abs(calculate_dot(plane.normal, center()) - plane.distance());

		// Compute the projection interval radius of this OBB onto L(t) = this->pos + x * p.normal;
		float t = halfWidth() * abs(calculate_dot(plane.normal,		CoordinateSystem::global_X())) +
				  halfHeight() * abs(calculate_dot(plane.normal,	CoordinateSystem::global_Y())) +
				  halfDepth() * abs(calculate_dot(plane.normal,		CoordinateSystem::global_Z()));

		/*
		Vec3 pMin = collider.normal() * min(); 
		Vec3 pMax = collider.normal() * max();

		if (pMin.x + pMax.y + pMax.z - collider.distance() < 0.f) return true;
		if (pMax.x + pMax.y + pMax.z - collider.distance() < 0.f) return true;
		if (pMin.x + pMax.y + pMin.z - collider.distance() < 0.f) return true;
		if (pMax.x + pMax.y + pMin.z - collider.distance() < 0.f) return true;

		if (pMin.x + pMin.y + pMax.z - collider.distance() < 0.f) return true;
		if (pMax.x + pMin.y + pMax.z - collider.distance() < 0.f) return true;
		if (pMin.x + pMin.y + pMin.z - collider.distance() < 0.f) return true;
		if (pMax.x + pMin.y + pMin.z - collider.distance() < 0.f) return true;
		
		return false;
		*/

		return s <= t;
	}

	bool		AABB::intersects(			const ConvexHull&	convexHull) const
	{
		for(auto& iFace : convexHull.faces())
		{
			if(above(iFace))
				return false;
		}

		return true;
	}
}