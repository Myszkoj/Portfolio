#pragma once


#include "cml_Plane.h"
#include <dpl_ReadOnly.h>
#include <dpl_GeneralException.h>
#include <vector>


namespace cml
{
	class Ray;
	class AABB;
	class OBB;
	class Sphere;
	class Cone;
	class ConvexHull;


	/*
		Set of planes forming solid shape.

		Plane normals must point outwards.

		If shape has no planes it is considered as infinite volume shape and thus collide with everything.
	*/
	class ConvexHull
	{
	public:
		using Faces = std::vector<Plane>;

	public: // data
		dpl::ReadOnly<Faces, ConvexHull> faces;

	public: // lifecycle
		CLASS_CTOR			ConvexHull()
		{
		}

		CLASS_CTOR			ConvexHull(		const Faces&			planes)
			: faces(planes)
		{
		}

		CLASS_CTOR			ConvexHull(		uint32_t				numPlanes)
			: faces(numPlanes)
		{
		}

		CLASS_CTOR			ConvexHull(		const Plane*			planes, 
											uint32_t				numPlanes)
			: faces(planes, planes + numPlanes)
		{
		}

	public: // reset functions
		inline void			reset(			const Faces&			planes)
		{
			faces = planes;
		}

		inline void			reset(			const Plane*			planes, 
											uint32_t				numPlanes)
		{
			faces->assign(planes, planes + numPlanes);
		}

		inline void			reset_face(		uint32_t				faceID,
											const Vec3&				normal,
											const float				distance)
		{
#ifdef _DEBUG
			if(faceID >= faces().size())
				throw dpl::GeneralException(this, __LINE__, "Invalid face ID: " + std::to_string(faceID));
#endif // _DEBUG

			(*faces)[faceID] = Plane(normal, distance);
		}

	public: // intersection functions
		bool				intersects(		const Vec3&				point) const;

		bool				intersects(		const AABB&				aabb) const;

		bool				intersects(		const OBB&				obb) const;

		bool				intersects(		const Sphere&			sphere) const;

		bool				intersects(		const Cone&				cone) const;

		bool				intersects(		const Plane&			plane) const;

		bool				intersects(		const ConvexHull&		other) const;
	};
}