#include "../include/cml_Ray.h"
#include "../include/cml_AABB.h"
#include "../include/cml_OBB.h"
#include "../include/cml_Sphere.h"
#include "../include/cml_Cone.h"
#include "../include/cml_Plane.h"
#include "../include/cml_ConvexHull.h"


namespace cml
{
	TriangleMesh	Cone::to_TriangleMesh(		const uint32_t		SLICES) const
	{
		TriangleMesh	output;

		if(SLICES > 1)
		{
			output.reserve_vertices(SLICES +1); // base vertices + apex
			output.reserve_indices(3 * SLICES + (SLICES-2)*3);

			//texCoords.resize(2 * slices +2);
			//normals.resize(2 * slices +2);

			const float deltaAngle	= ANGLE_360 / float(SLICES);
			const float radius		= calculate_radius();

			output.add_vertex(apex());

			//texCoords[APEX].s = 0.5f; // center of the texture
			//texCoords[APEX].t = 0.5f; //
			//normals[APEX].x = 0.f;
			//normals[APEX].y = 1.f; // normal points along the Y axis
			//normals[APEX].z = 0.f;

			const Vec3 baseCenter	= apex() + axis() * height();
			const Quat rot(CoordinateSystem::GLOBAL_FRONT, axis());

			for(uint32_t nSlice = 0; nSlice < SLICES; nSlice++)
			{
				const float	angle		= deltaAngle * float(nSlice);
				const Vec3	baseVertex	= Vec3(	radius * cos(angle),	// X
												radius * sin(angle),	// Y
												0.f);					// Z

				output.add_vertex(baseCenter + rot * baseVertex);

				// cone uv is mapped to circle projected on texture surface,
				// center of the circle is the center of a texture
				//texCoords[index].s = 0.5f + 0.5f * cos(angle); 
				//texCoords[index].t = 0.5f + 0.5f * sin(angle); 

				// calculate normal to current slice surface
				//glm::vec3 sliceNormal = glm::cross(positions[ORIGIN] - positions[index], positions[APEX] - positions[index]);

				//if(nSlice < slices)
				//	normals[index] = glm::cross(positions[ORIGIN] - positions[index], sliceNormal); // base normals
				//else
				//	normals[index] = glm::cross(sliceNormal, positions[APEX] - positions[index]);	// cone normals
			}

			// Add base indices.
			for(uint32_t i = 2; i < SLICES; ++i)
			{
				output.add_index(i+1);
				output.add_index(i); 
				output.add_index(1); // first vertex of the base
			}

			// Add cone indices.
			for(uint32_t i = SLICES, j = 1; j <= SLICES; i = j++)
			{
				output.add_index(i);
				output.add_index(j); 
				output.add_index(0); // apex vertex
			}
		}

		return output;
	}

	bool			Cone::above(				const Plane&		plane) const
	{
		const float apexDistance = calculate_dot(plane.normal(), this->apex());

		// Make sure that apex is above the plane.
		if(apexDistance > plane.distance())
		{
			// Calculate angle between plane normal and cone axis.
			const float cosTeta = calculate_dot(plane.normal(), this->axis());
			const float k		= glm::cos(ANGLE_90 - this->fi()) * calculate_radius();

			return apexDistance + cosTeta * this->height() - k > plane.distance();
		}

		return false;
	}

	bool			Cone::below(				const Plane&		plane) const
	{
		const float apexDistance = calculate_dot(plane.normal(), this->apex());

		// Make sure that apex is below the plane.
		if(apexDistance < plane.distance())
		{
			// Calculate angle between plane normal and cone axis.
			const float cosTeta = calculate_dot(plane.normal(), this->axis());
			const float k		= glm::cos(ANGLE_90 - this->fi()) * calculate_radius();

			return apexDistance + cosTeta * this->height() - k < plane.distance();
		}

		return false;
	}

	bool			Cone::intersects(			const Ray&			ray) const
	{
		throw dpl::GeneralException(this, __LINE__, "Cone-Ray test is not implemented yet.");
		return false;
	}

	bool			Cone::intersects(			const Vec3&			point) const
	{
		const Vec3	V	= point - this->apex(); 
		const float k	= glm::dot(this->axis(), V);

		// Project point on the cone axis.
		if(k < 0.f || k > this->height())
			return false;

		return (this->cosFi * this->cosFi) * glm::dot(V, V) <= k;
	}

	bool			Cone::intersects(			const AABB&			aabb) const
	{
		throw dpl::GeneralException(this, __LINE__, "Cone-AABB test is not implemented yet.");
		return false;
	}

	bool			Cone::intersects(			const OBB&			obb) const
	{
		throw dpl::GeneralException(this, __LINE__, "Cone-OBB test is not implemented yet.");
		return false;
	}

	bool			Cone::intersects(			const Sphere&		sphere) const
	{
		const Vec3	CmU		= sphere.center() - this->apex() + (sphere.radius() * this->sinFi()) * this->axis();
		const float AdCmU	= glm::dot(axis(), CmU);

		if(AdCmU > 0.f)
		{
			if(AdCmU * AdCmU >= glm::dot(CmU, CmU) * this->cosFi() * this->cosFi())
			{
				const Vec3 CmV = sphere.center() - this->apex();
				const float AdCmV = glm::dot(this->axis(), CmV);
				if(AdCmV < -sphere.radius())
					return false;

				if(AdCmV > this->height() + sphere.radius())
					return false;

				const float rSinAngle = sphere.radius() * this->sinFi();
				if(AdCmV >= -rSinAngle)
				{
					if(AdCmV <= this->height() - rSinAngle)
					{
						return true;
					}
					else
					{
						const Vec3	barD			= CmV - this->height() * this->axis();
						const float lengthAxBarD	= glm::length(glm::cross(this->axis(), barD));
						const float hTanAngle		= this->height() * (this->sinFi / cosFi);

						if(lengthAxBarD <= hTanAngle)
							return true;

						const float AdBarD	= AdCmV - this->height();
						const float diff	= lengthAxBarD - hTanAngle;

						return AdBarD * AdBarD + diff * diff <= sphere.radius() * sphere.radius();
					}
				}
				else
				{
					return glm::dot(CmV, CmV) <= sphere.radius() * sphere.radius();
				}
			}
		}

		return false;
	}

	bool			Cone::intersects(			const Cone&			other) const
	{
		throw dpl::GeneralException(this, __LINE__, "Cone-Cone test is not implemented yet.");
		return false;
	}

	bool			Cone::intersects(			const Plane&		plane) const
	{
		return plane.intersects(*this);
	}

	bool			Cone::intersects(			const ConvexHull&	convexHull) const
	{
		throw dpl::GeneralException(this, __LINE__, "Cone-ConvexHull test is not implemented yet.");
		return false;
	}
}