#include "../include/cml_Ray.h"
#include "../include/cml_AABB.h"
#include "../include/cml_OBB.h"
#include "../include/cml_Sphere.h"
#include "../include/cml_Cone.h"
#include "../include/cml_Plane.h"
#include "../include/cml_ConvexHull.h"


namespace cml
{
//=====> ConvexHull public: // intersection functions
	bool		ConvexHull::intersects(		const Vec3&			point) const
	{
		for(auto& iPlane : faces())
		{
			if(iPlane.point_above(point))
				return false;
		}
		
		return true;
	}

	bool		ConvexHull::intersects(		const AABB&			aabb) const
	{
		return aabb.intersects(*this);
	}

	bool		ConvexHull::intersects(		const OBB&			obb) const
	{
		return obb.intersects(*this);
	}

	bool		ConvexHull::intersects(		const Sphere&		sphere) const
	{
		for (auto& iFace : faces())
		{
			if(sphere.above(iFace))
				return false;
		}

		return true;
	}

	bool		ConvexHull::intersects(		const Cone&			cone) const
	{
		for(auto& iPlane : faces())
		{
			if(cone.above(iPlane))
				return false;
		}
		
		return true;
	}

	bool		ConvexHull::intersects(		const Plane&		plane) const
	{
		return plane.intersects(*this);
	}

	bool		ConvexHull::intersects(		const ConvexHull&	other) const
	{
		throw dpl::GeneralException(this, __LINE__, "This function is not implemented yet.");
		return false;
	}
}