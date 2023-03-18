#include "../include/cml_Ray.h"
#include "../include/cml_AABB.h"
#include "../include/cml_OBB.h"
#include "../include/cml_Sphere.h"
#include "../include/cml_Cone.h"
#include "../include/cml_Plane.h"
#include "../include/cml_ConvexHull.h"


namespace cml
{
	inline float	project_scale(	const Mat3& mat, 
									const Vec3& axis)
	{
		return abs(calculate_dot(axis, mat[0]))
			 + abs(calculate_dot(axis, mat[1]))
			 + abs(calculate_dot(axis, mat[2]));
	}


	CLASS_CTOR	OBB::OBB()
		: Cuboid(0.f, 0.f, 0.f)
	{

	}

	CLASS_CTOR	OBB::OBB(					const AABB&				aabb)
		: Cuboid(aabb)
		, CoordinateSystem(aabb.center, CoordinateSystem::global_X(), CoordinateSystem::global_Y())
	{

	}

	CLASS_CTOR	OBB::OBB(					const AABB&				aabb,
											const Mat4&				TRANSFORMATION)
		: Cuboid(aabb)
		, CoordinateSystem(aabb.center())
	{
		Vec3 scale;
		CoordinateSystem::reset(TRANSFORMATION, scale);
		CoordinateSystem::move_origin(aabb.center());
		Cuboid::scale(scale);
	}

	void		OBB::reset(					const AABB&				aabb)
	{
		Cuboid::set_extents(aabb.extents());
		set_origin(aabb.center());
		reset_axes(CoordinateSystem::global_X(), CoordinateSystem::global_Y(), CoordinateSystem::global_Z());
	}

	void		OBB::reset(					const AABB&				aabb,
											const Mat4&				TRANSFORMATION)
	{
		Vec3 scale;
		CoordinateSystem::reset(TRANSFORMATION, scale);
		CoordinateSystem::move_origin(aabb.center());
		Cuboid::set_extents(aabb.halfWidth() * scale.x,
							aabb.halfHeight() * scale.y,
							aabb.halfDepth() * scale.z);
	}

	void		OBB::transform(				const Mat4&				TRANSFORMATION)
	{
		Vec3 scale;
		CoordinateSystem::transform(TRANSFORMATION, scale);
		Cuboid::scale(scale);
	}

	Cuboid		OBB::align_size() const
	{
		Mat3 M(	front()	* halfWidth(),
				up()	* halfHeight(),
				right()	* halfDepth());

		return Cuboid(	project_scale(M, CoordinateSystem::global_X()) * 2.f,
						project_scale(M, CoordinateSystem::global_Y()) * 2.f,
						project_scale(M, CoordinateSystem::global_Z()) * 2.f);
	}

	Vec3		OBB::corner(				Corner					cornerID) const
	{
		switch(cornerID)
		{
		default: // Should anything go wrong, we return the first case.
		case eLEFT_BOTTOM_BACK:		return center()	-halfWidth() * local_X() -halfHeight() * local_Y() -halfDepth() * local_Z();
		case eLEFT_BOTTOM_FRONT:	return center()	-halfWidth() * local_X() -halfHeight() * local_Y() +halfDepth() * local_Z();
		case eLEFT_TOP_BACK:		return center()	-halfWidth() * local_X() +halfHeight() * local_Y() -halfDepth() * local_Z();
		case eLEFT_TOP_FRONT:		return center()	-halfWidth() * local_X() +halfHeight() * local_Y() +halfDepth() * local_Z();
		case eRIGHT_BOTTOM_BACK:	return center()	+halfWidth() * local_X() -halfHeight() * local_Y() -halfDepth() * local_Z();
		case eRIGHT_BOTTOM_FRONT:	return center()	+halfWidth() * local_X() -halfHeight() * local_Y() +halfDepth() * local_Z();
		case eRIGHT_TOP_BACK:		return center()	+halfWidth() * local_X() +halfHeight() * local_Y() -halfDepth() * local_Z();
		case eRIGHT_TOP_FRONT:		return center()	+halfWidth() * local_X() +halfHeight() * local_Y() +halfDepth() * local_Z();
		}
	}

	Vec3		OBB::face_center(			Face					faceID) const
	{
		switch(faceID)
		{
		default: // Should anything go wrong, we return the first case.
		case eLEFT:		return center()	-halfWidth() * local_X();
		case eRIGHT:	return center()	+halfWidth() * local_X();
		case eBOTTOM:	return center()	-halfHeight() * local_Y();
		case eTOP:		return center()	+halfHeight() * local_Y();
		case eBACK:		return center()	-halfDepth() * local_Z();
		case eFRONT:	return center()	+halfDepth() * local_Z();
		}
	}

	Vec3		OBB::face_normal(			Face					faceID) const
	{
		switch(faceID)
		{
		default: // Should anything go wrong, we return the first case.
		case eLEFT:		return -local_X();
		case eRIGHT:	return local_X();
		case eBOTTOM:	return -local_Y();
		case eTOP:		return local_Y();
		case eBACK:		return -local_Z();
		case eFRONT:	return local_Z();
		}
	}
		
	Plane		OBB::face_plane(			Face					faceID) const
	{
		return Plane(face_center(faceID), face_normal(faceID));
	}

	Vec3		OBB::closest_point(			const Vec3&				point) const
	{
		Vec3 rel = point - origin();

		return origin() + glm::clamp(calculate_dot(rel, front()), -halfWidth(), halfWidth()) * front()
						+ glm::clamp(calculate_dot(rel, up()), -halfHeight(), halfHeight()) * up()
						+ glm::clamp(calculate_dot(rel, right()), -halfDepth(), halfDepth()) * right();
	}

	bool		OBB::intersects(			const Vec3&				point) const
	{
		Vec3 rel = point - origin();

		if(abs(calculate_dot(front(), rel)) > halfWidth())
			return false;

		if(abs(calculate_dot(up(), rel)) > halfHeight())
			return false;

		if(abs(calculate_dot(right(), rel)) > halfDepth())
			return false;

		return true;
	}

	bool		OBB::intersects(			const AABB&				aabb) const
	{
		return intersects(OBB(aabb));
	}

	bool		OBB::intersects(			const OBB&				OTHER) const
	{
		// https://www.geometrictools.com/Documentation/DynamicCollisionDetection.pdf
		// https://www.randygaul.net/2014/05/22/deriving-obb-to-obb-intersection-sat/

		/*
		// Generate a rotation matrix that transforms from world space to this OBB's coordinate space.
		Mat3 R;
		for(int i = 0; i < 3; ++i)
			for(int j = 0; j < 3; ++j)
				R[i][j] = calculate_dot(axes()[i], OTHER.axes()[j]);

		Vec3	t = OTHER.origin() - origin();
				t.x = calculate_dot(t, X());
				t.y = calculate_dot(t, Y());
				t.z = calculate_dot(t, Z());

		Mat3 AbsR;
		for(int i = 0; i < 3; ++i)
			for(int j = 0; j < 3; ++j)
				AbsR[i][j] = abs(R[i][j]); // removed + epsilon

		// Test the three major axes of this OBB.
		for(int i = 0; i < 3; ++i)
		{
			float ra = R[i];
			float rb = DOT3(b.r, AbsR[i]);
			if (abs(t[i]) > ra + rb)
				return false;
		}

		// Test the three major axes of the OBB b.
		for(int i = 0; i < 3; ++i)
		{
			float ra = r[0] * AbsR[0][i] + r[1] * AbsR[1][i] + r[2] * AbsR[2][i];
			float rb = b.r[i];
			if (abs(t.x * R[0][i] + t.y * R[1][i] + t.z * R[2][i]) > ra + rb)
				return false;
		}

		// Test the 9 different cross-axes.

		// A.x <cross> B.x
		float ra = r.y * AbsR[2][0] + r.z * AbsR[1][0];
		float rb = b.r.y * AbsR[0][2] + b.r.z * AbsR[0][1];
		if (abs(t.z * R[1][0] - t.y * R[2][0]) > ra + rb)
			return false;

		// A.x < cross> B.y
		ra = r.y * AbsR[2][1] + r.z * AbsR[1][1];
		rb = b.r.x * AbsR[0][2] + b.r.z * AbsR[0][0];
		if (abs(t.z * R[1][1] - t.y * R[2][1]) > ra + rb)
			return false;

		// A.x <cross> B.z
		ra = r.y * AbsR[2][2] + r.z * AbsR[1][2];
		rb = b.r.x * AbsR[0][1] + b.r.y * AbsR[0][0];
		if (abs(t.z * R[1][2] - t.y * R[2][2]) > ra + rb)
			return false;

		// A.y <cross> B.x
		ra = r.x * AbsR[2][0] + r.z * AbsR[0][0];
		rb = b.r.y * AbsR[1][2] + b.r.z * AbsR[1][1];
		if (abs(t.x * R[2][0] - t.z * R[0][0]) > ra + rb)
			return false;

		// A.y <cross> B.y
		ra = r.x * AbsR[2][1] + r.z * AbsR[0][1];
		rb = b.r.x * AbsR[1][2] + b.r.z * AbsR[1][0];
		if (abs(t.x * R[2][1] - t.z * R[0][1]) > ra + rb)
			return false;

		// A.y <cross> B.z
		ra = r.x * AbsR[2][2] + r.z * AbsR[0][2];
		rb = b.r.x * AbsR[1][1] + b.r.y * AbsR[1][0];
		if (abs(t.x * R[2][2] - t.z * R[0][2]) > ra + rb)
			return false;

		// A.z <cross> B.x
		ra = r.x * AbsR[1][0] + r.y * AbsR[0][0];
		rb = b.r.y * AbsR[2][2] + b.r.z * AbsR[2][1];
		if (abs(t.y * R[0][0] - t.x * R[1][0]) > ra + rb)
			return false;

		// A.z <cross> B.y
		ra = r.x * AbsR[1][1] + r.y * AbsR[0][1];
		rb = b.r.x * AbsR[2][2] + b.r.z * AbsR[2][0];
		if (abs(t.y * R[0][1] - t.x * R[1][1]) > ra + rb)
			return false;

		// A.z <cross> B.z
		ra = r.x * AbsR[1][2] + r.y * AbsR[0][2];
		rb = b.r.x * AbsR[2][1] + b.r.y * AbsR[2][0];
		if (abs(t.y * R[0][2] - t.x * R[1][2]) > ra + rb)
			return false;

		// No separating axis exists, so the two OBB don't intersect.
		return true;
		*/

		/*
		const Vec3 TO_OTHER = OTHER.center() - this->center();

		for(uint32_t axisID = 0; axisID < 3; ++axisID)
		{
			const Vec3& THIS_AXIS		= this->get_axis(axisID);
			const Vec3& OTHER_AXIS		= OTHER.get_axis(axisID);

			const float THIS_EXTENT		= this->get_extent(axisID);
			const float OTHER_EXTENT	= OTHER.get_extent(axisID);

			if(abs(glm::dot(THIS_AXIS, TO_OTHER)) > THIS_EXTENT + OTHER.project_size(THIS_AXIS))
				return false;

			if(abs(glm::dot(OTHER_AXIS, TO_OTHER)) > OTHER_EXTENT + this->project_size(OTHER_AXIS))
				return false;
		}

		return true;
		*/

		const Vec3 TO_OTHER = OTHER.center() - this->center();

		for(uint32_t firstID = 0; firstID < 3; ++firstID)
		{
			const Vec3& THIS_AXIS = this->get_axis(firstID);

			if(is_separation_axis(OTHER, TO_OTHER, THIS_AXIS)
			|| is_separation_axis(OTHER, TO_OTHER, OTHER.get_axis(firstID)))
				return false;

			for(uint32_t secondID = 0; secondID < 3; ++secondID)
			{
				if(is_separation_axis(OTHER, TO_OTHER, glm::cross(THIS_AXIS, OTHER.get_axis(secondID))))
					return false;
			}
		}

		return true;
	}

	bool		OBB::intersects(			const Sphere&			sphere) const
	{
		Vec3 L = closest_point(sphere.center()) - sphere.center();

		return calculate_dot(L, L) <= sphere.radius() * sphere.radius();
	}

	bool		OBB::intersects(			const Cone&				cone) const
	{
		return cone.intersects(*this);
	}

	bool		OBB::intersects(			const Plane&			plane) const
	{
		return abs(calculate_dot(plane.normal, origin()) - plane.distance()) <= project_size(plane.normal);
	}

	bool		OBB::intersects(			const ConvexHull&		convexHull) const
	{
		for(auto& iFace : convexHull.faces())
		{
			// Compute the distance of this OBB center from the plane.
			float s = calculate_dot(iFace.normal, origin()) - iFace.distance();
			if(s < 0.f)
				continue; // Center is inside the convex hull.

			// Compute the projection interval radius of this OBB onto L(t) = this->pos + x * p.normal;
			float t = project_size(iFace.normal); // "Half size"?
		
			if(s > t)
				return false;
		}

		return true;
	}

	void		OBB::extend(				const Vec3&				POINT)
	{
		const Vec3 CENTER_TO_POINT = POINT - center();

		set_extents(  glm::max(halfWidth(), abs(calculate_dot(CENTER_TO_POINT, front())))
					, glm::max(halfHeight(), abs(calculate_dot(CENTER_TO_POINT, up())))
					, glm::max(halfDepth(), abs(calculate_dot(CENTER_TO_POINT, right()))));
	}

	void		OBB::extend(				const OBB&				OTHER)
	{
		set_extents(  glm::max(halfWidth(), OTHER.project_size(front()))
					, glm::max(halfHeight(), OTHER.project_size(up()))
					, glm::max(halfDepth(), OTHER.project_size(right())));
	}

	bool		OBB::is_separation_axis(	const OBB&				OTHER,
											const Vec3&				TO_OTHER,
											const Vec3&				AXIS) const
	{
		const float LHV = abs(glm::dot(TO_OTHER, AXIS));
		const float RHV = abs(glm::dot(this->local_X() * this->halfWidth(),		AXIS))
						+ abs(glm::dot(this->local_Y() * this->halfHeight(),	AXIS))
						+ abs(glm::dot(this->local_Z() * this->halfDepth(),		AXIS))
						+ abs(glm::dot(OTHER.local_X() * OTHER.halfWidth(),		AXIS))
						+ abs(glm::dot(OTHER.local_Y() * OTHER.halfHeight(),	AXIS))
						+ abs(glm::dot(OTHER.local_Z() * OTHER.halfDepth(),		AXIS));

		return	LHV > RHV;
	}
}