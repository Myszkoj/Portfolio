#include "../include/cml_Ray.h"
#include "../include/cml_AABB.h"
#include "../include/cml_OBB.h"
#include "../include/cml_Sphere.h"
#include "../include/cml_Cone.h"
#include "../include/cml_Plane.h"
#include "../include/cml_ConvexHull.h"


namespace cml
{
	CLASS_CTOR		Sphere::Sphere(				const AABB&			aabb)
		: center(aabb.center())
		, radius(aabb.range())
	{
	}

	TriangleMesh	Sphere::to_TriangleMesh(	const uint32_t		MERIDIANS, 
												const uint32_t		PARALLELS) const
	{
		TriangleMesh	output;
				output.reserve_vertices(2 + MERIDIANS*(PARALLELS-1));
				output.reserve_indices(2*((MERIDIANS-2)*PARALLELS + PARALLELS));

		// first stack
		output.add_vertex(Vec3(0.f, 0.f, radius()) + center());

		// angles
		const float deltaStackAngle	= glm::pi<float>() / (MERIDIANS -1); // alpha
		const float deltaSliceAngle	= (2 * glm::pi<float>()) / PARALLELS; // beta
		float stackAngle	= deltaStackAngle; // we already added first stack
		float sliceAngle	= 0.0;

		// create vertices for each layer
		for(uint32_t n = 1; n < MERIDIANS -1; ++n)
		{
			const float layerRadius = radius() * sin(stackAngle);

			Vec3 offset(0.f, 0.f, radius() * cos(stackAngle));
		
			// angle between X axis and current layer vertex
			sliceAngle = 0.0;
			for(uint32_t m = 0; m < PARALLELS; m++)
			{
				offset.x = (float)(layerRadius * cos(sliceAngle));
				offset.y = (float)(layerRadius * sin(sliceAngle));

				// calculate UV mapping
				output.add_vertex(offset + center());

				sliceAngle += deltaSliceAngle;
			}

			stackAngle += deltaStackAngle;
		}

		// last layer
		output.add_vertex(Vec3(0.f, 0.f, -radius()) + center());

		// connection between first two stacks
		for(uint32_t m = 0; m < PARALLELS; ++m)
		{
			uint32_t i0 = 0;
			uint32_t i1 = m +1;
			uint32_t i2 = +1;

			if(m < PARALLELS -1)
				i2 = m +2;

			output.add_index(i0);
			output.add_index(i1);
			output.add_index(i2);
		} 

		// connection between middle layers
		for(uint32_t n = 1; n < MERIDIANS -2; ++n)
		{
			unsigned int indexOffset = 1;
			if(n > 0)
				indexOffset += (n-1) * PARALLELS;
			
			for(uint32_t m = 0; m < PARALLELS; ++m)
			{
				uint32_t i0 = indexOffset +m;
				uint32_t i1 = indexOffset +m +PARALLELS; 
				uint32_t i2 = indexOffset +PARALLELS;
				uint32_t i3 = indexOffset +m;
				uint32_t i4 = indexOffset +PARALLELS;
				uint32_t i5 = indexOffset;

				if(m < PARALLELS -1) 
					i2 = indexOffset +m +PARALLELS +1;

				if(m < PARALLELS -1)
				{
					i4 = indexOffset +m +PARALLELS +1;
					i5 = indexOffset +m +1;				
				}

				///////////////////////////////////////
				output.add_index(i0);
				output.add_index(i1);
				output.add_index(i2);
				output.add_index(i3);
				output.add_index(i4);
				output.add_index(i5);		
			}
			
		}

		// connection between last two stacks
		for(uint32_t m = 0; m < PARALLELS; ++m)
		{
			const uint32_t indexOffset = 1 + PARALLELS * (MERIDIANS -3);
			const uint32_t i0 = output.get_numVertices() -1;
			const uint32_t i1 = m < PARALLELS -1 ? indexOffset + m +1 : indexOffset;
			const uint32_t i2 = indexOffset +m;
			
			output.add_index(i0);		
			output.add_index(i1);
			output.add_index(i2);
		}

		return output;
	}

	bool			Sphere::contains(			const AABB&			aabb) const
	{
		for(uint32_t i = 0; i < 8; ++i)
		{
			if(!contains(aabb.corner(static_cast<Cuboid::Corner>(i))))
				return false;
		}

		return true;
	}

	bool			Sphere::contains(			const OBB&			obb) const
	{
		for(uint32_t i = 0; i < 8; ++i)
		{
			if(!contains(obb.corner(static_cast<Cuboid::Corner>(i))))
				return false;
		}

		return true;
	}

	bool			Sphere::above(				const Plane&		plane) const
	{
		return calculate_dot(plane.normal(), center()) - radius() > plane.distance();
	}

	bool			Sphere::below(				const Plane&		plane) const
	{
		return calculate_dot(plane.normal(), center()) + radius() < plane.distance();
	}

	bool			Sphere::intersects(			const Ray&			ray) const
	{
		Vec3 toCenter = center() - ray.origin();

		// Make sure, that ray is outside the sphere.
		if(calculate_length(toCenter) < radius())
			return false;

		// Project 'toCenter' on the plane perpendicular to ray direction and compare to sphere radius.
		return abs(calculate_det(ray.direction(), toCenter)) < radius();
	}

	bool			Sphere::intersects(			const Vec3&			point) const
	{
		return calculate_distance(point, center()) < radius();
	}

	bool			Sphere::intersects(			const AABB&			aabb) const
	{
		float r2 = radius() * radius();

		const Vec3 min = aabb.min();
		const Vec3 max = aabb.max();

		if (center().x < min.x)
			r2 -= glm::pow(center().x - min.x, 2.f);
		else if (center().x > max.x)
			r2 -= glm::pow(center().x - max.x, 2.f);

		if (center().y < min.y)
			r2 -= glm::pow(center().y - min.y, 2.f);
		else if (center().y > max.y)
			r2 -= glm::pow(center().y - max.y, 2.f);

		if (center().z < min.z)
			r2 -= glm::pow(center().z - min.z, 2.f);
		else if (center().z > max.z)
			r2 -= glm::pow(center().z - max.z, 2.f);

		return r2 > 0.f;
	}

	bool			Sphere::intersects(			const OBB&			obb) const
	{
		return obb.intersects(*this);
	}

	bool			Sphere::intersects(			const Sphere&		other) const
	{
		return calculate_distance(this->center(), other.center()) < this->radius() + other.radius();
	}

	bool			Sphere::intersects(			const Cone&			cone) const
	{
		return cone.intersects(*this);
	}

	bool			Sphere::intersects(			const Plane&		plane) const
	{
		return abs(calculate_dot(center(), plane.normal()) - plane.distance()) < radius();
	}

	bool			Sphere::intersects(			const ConvexHull&	convexHull) const
	{
		return convexHull.intersects(*this);
	}
}