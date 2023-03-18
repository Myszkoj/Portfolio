#pragma once


#include "cml_Cuboid.h"


namespace cml
{
	class Ray;
	class AABB;
	class OBB;
	class Sphere;
	class Cone;
	class Plane;
	class ConvexHull;



	class	AABB : public Cuboid
	{
	public: // data
		dpl::ReadOnly<Vec3, AABB> center;

	public: // lifecycle
		CLASS_CTOR			AABB()
			: center(0.f, 0.f, 0.f)
		{
			
		}

		CLASS_CTOR			AABB(				const Vec3&			CENTER)
			: center(CENTER)
		{
			
		}

		CLASS_CTOR			AABB(				const Vec3&			min, 
												const Vec3&			max);

		CLASS_CTOR			AABB(				const Cuboid&		cuboid, 
												const Vec3&			center);

		CLASS_CTOR			AABB(				const OBB&			obb);

		CLASS_CTOR			AABB(				const Sphere&		sphere);

		CLASS_CTOR			AABB(				const Vec3*			POINTS,
												const uint32_t		NUM_POINTS);

		CLASS_CTOR			AABB(				const Vec3*			POINTS,
												const uint8_t*		INDICES,
												const uint32_t		NUM_INDICES);

		CLASS_CTOR			AABB(				const Vec3*			POINTS,
												const uint16_t*		INDICES,
												const uint32_t		NUM_INDICES);

		CLASS_CTOR			AABB(				const Vec3*			POINTS,
												const uint32_t*		INDICES,
												const uint32_t		NUM_INDICES);

	public: // comparsion
		inline bool			operator==(			const AABB&			OTHER) const
		{
			return Cuboid::operator==(OTHER) && (this->center() == OTHER.center());
		}

		inline bool			operator!=(			const AABB&			OTHER) const
		{
			return Cuboid::operator!=(OTHER) || (this->center() != OTHER.center());
		}

	public: // functions
		OBB					operator*(			const Mat4&			TRANSFORMATION) const;

		void				reset(				const Vec3&			min, 
												const Vec3&			max);

		void				reset(				const Vec3*			POINTS,
												const uint32_t		numPoints);

		void				reset(				const Vec3*			POINTS,
												const uint8_t*		indices,
												const uint32_t		numIndices);

		void				reset(				const Vec3*			POINTS,
												const uint16_t*		indices,
												const uint32_t		numIndices);

		void				reset(				const Vec3*			POINTS,
												const uint32_t*		indices,
												const uint32_t		numIndices);

		inline void			set_center(			const Vec3&			NEW_CENTER)
		{
			center = NEW_CENTER;
		}

		inline void			set_center(			const float			NEW_X,
												const float			NEW_Y,
												const float			NEW_Z)
		{
			center->x = NEW_X;
			center->y = NEW_Y;
			center->z = NEW_Z;
		}

		inline float		min_x() const
		{
			return center().x - halfWidth();
		}

		inline float		min_y() const
		{
			return center().y - halfHeight();
		}

		inline float		min_z() const
		{
			return center().z - halfDepth();
		}

		inline float		max_x() const
		{
			return center().x + halfWidth();
		}

		inline float		max_y() const
		{
			return center().y + halfHeight();
		}

		inline float		max_z() const
		{
			return center().z + halfDepth();
		}

		inline Vec3			min() const
		{
			return Vec3(min_x(), min_y(), min_z());
		}

		inline Vec3			max() const
		{
			return Vec3(max_x(), max_y(), max_z());
		}

		Vec3				corner(				const Corner		CORNER_ID) const;

		Vec3				face_center(		const Face			FACE_ID) const;

		Vec3				face_normal(		const Face			FACE_ID) const;

		Plane				face_plane(			const Face			FACE_ID) const;

		Vec3				closest_point(		const Vec3&			point) const;

		inline float		project_size(		const Vec3&			axis) const
		{
			return abs(calculate_dot(axis, CoordinateSystem::global_X()) * halfWidth())
				 + abs(calculate_dot(axis, CoordinateSystem::global_Y()) * halfHeight())
				 + abs(calculate_dot(axis, CoordinateSystem::global_Z()) * halfDepth());
		}

		inline void			move(				const Vec3&			offset)
		{
			*center += offset;
		}

		void				extend(				const Vec3&			point);

		void				extend(				const Vec3&			min, 
												const Vec3&			max);

		inline void			extend(				const AABB&			other)
		{
			extend(other.min(), other.max());
		}

		float				distance(			const Vec3&			point) const;

		float				distance(			const Sphere&		sphere) const;

		bool				contains(			const Vec3&			point) const;

		bool				contains(			const AABB&			other) const;

		inline bool			contains(			const OBB&			obb) const
		{
			return contains(AABB(obb));
		}

		inline bool			contains(			const Sphere&		sphere) const
		{
			return contains(AABB(sphere));
		}

		bool				above(				const Plane&		plane) const;

		bool				below(				const Plane&		plane) const;

		bool				intersects(			const Ray&			ray) const;

		bool				intersects(			const Vec3&			point) const;

		bool				intersects(			const AABB&			other) const;

		bool				intersects(			const OBB&			obb) const;

		bool				intersects(			const Sphere&		sphere) const;

		bool				intersects(			const Cone&			cone) const;

		bool				intersects(			const Plane&		plane) const;

		bool				intersects(			const ConvexHull&	convexHull) const;
	};
}