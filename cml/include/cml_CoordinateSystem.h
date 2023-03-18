#pragma once


#include <dpl_ReadOnly.h>
#include <dpl_GeneralException.h>
#include "cml_utilities.h"


namespace cml
{
	class CoordinateSystem
	{
	public: // subtypes
		enum Axis
		{
			X_AXIS	= 0,
			Y_AXIS	= 1,
			Z_AXIS	= 2
		};

		enum Direction
		{
			FRONT	= X_AXIS,
			UP		= Y_AXIS,
			RIGHT	= Z_AXIS
		};

	public: // constants
		static const Vec3 GLOBAL_ORIGIN;
		static const Mat3 GLOBAL_AXES;
		static const Vec3 GLOBAL_FRONT;
		static const Vec3 GLOBAL_BACK;
		static const Vec3 GLOBAL_UP;
		static const Vec3 GLOBAL_DOWN;
		static const Vec3 GLOBAL_RIGHT;
		static const Vec3 GLOBAL_LEFT;

	private: // data
		dpl::ReadOnly<glm::mat4x3, CoordinateSystem> data;

	public: // lifecycle
		CLASS_CTOR			CoordinateSystem()
			: data(global_X(), global_Y(), global_Z(), GLOBAL_ORIGIN)
		{

		}

		CLASS_CTOR			CoordinateSystem(		const Vec3&				ORIGIN)
			: data(global_X(), global_Y(), global_Z(), ORIGIN)
		{

		}

		CLASS_CTOR			CoordinateSystem(		const Mat4&				TRANSFORMATION)
			: CoordinateSystem()
		{
			reset(TRANSFORMATION);
		}

		CLASS_CTOR			CoordinateSystem(		const Vec3&				ORIGIN,
													const Mat3&				AXES)
			: data(AXES[0], AXES[1], AXES[2], ORIGIN)
		{

		}

		CLASS_CTOR			CoordinateSystem(		const Vec3&				ORIGIN,
													const Vec3&				X,
													const Vec3&				Y,
													const Vec3&				Z)
			: data(X, Y, Z, ORIGIN)
		{

		}

		CLASS_CTOR			CoordinateSystem(		const Vec3&				ORIGIN,
													const Quat&				ORIENTATION)
			: data(ORIENTATION * global_X(), ORIENTATION * global_Y(), ORIENTATION * global_Z(), ORIGIN)
		{

		}

		CLASS_CTOR			CoordinateSystem(		const Vec3&				ORIGIN,
													const Vec3&				FRONT,
													const Vec3&				UP)
			: data(FRONT, normalize_vector(calculate_cross(FRONT, UP)), calculate_cross(right(), FRONT), ORIGIN)
		{

		}

	public: // operators
		inline bool			operator==(				const CoordinateSystem& OTHER) const
		{
			return data() == OTHER.data();
		}

		inline bool			operator!=(				const CoordinateSystem& OTHER) const
		{
			return data() != OTHER.data();
		}

	public: // global axes
		static const Vec3&	global_X()
		{
			return GLOBAL_AXES[X_AXIS];
		}

		static const Vec3&	global_Y()
		{
			return GLOBAL_AXES[Y_AXIS];
		}

		static const Vec3&	global_Z()
		{
			return GLOBAL_AXES[Z_AXIS];
		}

	public: // local axes
		inline const Vec3&	local_X() const
		{
			return data()[X_AXIS];
		}

		inline const Vec3&	local_Y() const
		{
			return data()[Y_AXIS];
		}

		inline const Vec3&	local_Z() const
		{
			return data()[Z_AXIS];
		}

		inline const Mat3&	axes() const
		{
			return reinterpret_cast<const Mat3&>(data());
		}

		inline const Vec3&	get_axis(				const uint32_t			AXIS_INDEX) const
		{
			validate_axis(AXIS_INDEX);
			return data()[AXIS_INDEX];
		}

		inline void			flip_axis(				const uint32_t			AXIS_INDEX)
		{
			validate_axis(AXIS_INDEX);
			(*data)[AXIS_INDEX] *= -1.f;
		}

		inline void			flip_X()
		{
			flip_axis(0);
		}

		inline void			flip_Y()
		{
			flip_axis(1);
		}

		inline void			flip_Z()
		{
			flip_axis(2);
		}

		inline void			reset_axes(				const Vec3&				X,
													const Vec3&				Y,
													const Vec3&				Z)
		{
			(*data)[0] = X;
			(*data)[1] = Y;
			(*data)[2] = Z;
		}

		inline void			reset_axes(				const Quat&				ORIENTATION)
		{
			(*data)[0] = ORIENTATION * global_X();
			(*data)[1] = ORIENTATION * global_Y();
			(*data)[2] = ORIENTATION * global_Z();
		}

		inline void			reset_X(				const Vec3&				Y,
													const Vec3&				Z)
		{
			(*data)[0] = normalize_vector(calculate_cross(Y, Z));
			(*data)[1] = Y;
			(*data)[2] = Z;
		}

		inline void			reset_Y(				const Vec3&				X,
													const Vec3&				Z)
		{
			(*data)[0] = X;
			(*data)[1] = normalize_vector(calculate_cross(Z, X));
			(*data)[2] = Z;
		}

		inline void			reset_Z(				const Vec3&				X,
													const Vec3&				Y)
		{
			(*data)[0] = X;
			(*data)[1] = Y;
			(*data)[2] = normalize_vector(calculate_cross(X, Y));
		}

		void				reset(					const Mat4&				TRANSFORMATION,
													Vec3&					scale);

		inline void			reset(					const Mat4&				TRANSFORMATION)
		{
			static glm::vec3 dummy(1.f, 1.f, 1.f);
			reset(TRANSFORMATION, dummy);
		}

		inline void			reset(					const CoordinateSystem& OTHER)
		{
			if(this != &OTHER)
			{
				data = OTHER.data;
			}
		}

		inline void			transform(				const Mat4&				TRANSFORMATION,
													Vec3&					scale)
		{
			reset(TRANSFORMATION * Mat4(data()), scale);
		}

		inline void			transform(				const Mat4&				TRANSFORMATION)
		{
			reset(TRANSFORMATION * Mat4(data()));
		}

	public: // directional axes
		inline const Vec3&	front() const
		{
			return data()[FRONT];
		}

		inline const Vec3&	up() const
		{
			return data()[UP];
		}

		inline const Vec3&	right() const
		{
			return data()[RIGHT];
		}

	public: // distances
		inline float		front_distance(			const glm::vec3&		POINT) const
		{
			return glm::dot(front(), POINT-origin());
		}

		inline float		up_distance(			const glm::vec3&		POINT) const
		{
			return glm::dot(up(), POINT-origin());
		}

		inline float		right_distance(			const glm::vec3&		POINT) const
		{
			return glm::dot(right(), POINT-origin());
		}

	public: // origin
		inline const Vec3&	origin() const
		{
			return data()[3];
		}

		inline const Vec3&	center() const
		{
			return data()[3];
		}

		inline void			set_origin(				const float				pX,
													const float				pY,
													const float				pZ)
		{
			auto&	origin = (*data)[3];
					origin.x = pX;
					origin.y = pY;
					origin.z = pZ;
		}

		inline void			set_origin(				const Vec3&				NEW_ORIGIN)
		{
			(*data)[3] = NEW_ORIGIN;
		}

		inline void			move_origin(			const float				dX,
													const float				dY,
													const float				dZ)
		{
			auto&	origin = (*data)[3];
					origin.x += dX;
					origin.y += dY;
					origin.z += dZ;
		}

		inline void			move_origin(			const Vec3&				OFFSET)
		{
			set_origin(origin() + OFFSET);
		}

	public: // rotation
		inline void			rotate_around_X(		const float				RADIANS)
		{
			(*data)[1]	= Rodrigues_rotation(local_Y(), local_X(), RADIANS);
			(*data)[2]	= normalize_vector(calculate_cross(local_X(), local_Y()));
		}

		inline void			rotate_around_Y(		const float				RADIANS)
		{
			(*data)[0]	= Rodrigues_rotation(local_X(), local_Y(), RADIANS);
			(*data)[2]	= normalize_vector(calculate_cross(local_X(), local_Y()));
		}

		inline void			rotate_around_Z(		const float				RADIANS)
		{
			(*data)[1]	= Rodrigues_rotation(local_Y(), local_Z(), RADIANS);
			(*data)[0]	= normalize_vector(calculate_cross(local_Y(), local_Z()));
		}

		inline void			rotate(					const Quat&				ROTATION)
		{
			(*data)[0]	= ROTATION * local_X();
			(*data)[1]	= ROTATION * local_Y();
			(*data)[2]	= ROTATION * local_Z();
		}

		inline void			rotate(					const Quat&				ROTATION,
													const Vec3&				ORIGIN)
		{
			set_origin(ORIGIN + ROTATION * (this->origin() - ORIGIN));
			rotate(ROTATION);
		}

		inline void			rotate_around_X(		const CoordinateSystem& OTHER,
													const float				RADIANS)
		{
			rotate(glm::angleAxis(RADIANS, OTHER.local_X()), OTHER.origin());
		}

		inline void			rotate_around_Y(		const CoordinateSystem& OTHER,
													const float				RADIANS)
		{
			rotate(glm::angleAxis(RADIANS, OTHER.local_Y()), OTHER.origin());
		}

		inline void			rotate_around_Z(		const CoordinateSystem& OTHER,
													const float				RADIANS)
		{
			rotate(glm::angleAxis(RADIANS, OTHER.local_Z()), OTHER.origin());
		}

		inline void			align_with(				const glm::vec3&		TARGET,
													const float				DISTANCE)
		{
			const auto NEW_POSITION        = TARGET - front() * DISTANCE;
			const auto OFFSET_TO_POSITION  = NEW_POSITION - origin();
			move_origin(OFFSET_TO_POSITION);
		}

		void				turn_towards(			const glm::vec3&		TARGET);

	public: // tilt functions
		inline float		get_front_tilt(			const Vec3&				REFERENCE_AXIS) const
		{
			return calculate_dot(front(), REFERENCE_AXIS);
		}

		inline float		get_up_tilt(			const Vec3&				REFERENCE_AXIS) const
		{
			return calculate_dot(up(), REFERENCE_AXIS);
		}

		inline float		get_right_tilt(			const Vec3&				REFERENCE_AXIS) const
		{
			return calculate_dot(right(), REFERENCE_AXIS);
		}

	public: // to-view transformation
		void				to_matrix(				Mat4&					view) const;

		void				to_view_matrix(			Mat4&					view) const;

		inline Mat4			to_matrix() const
		{
			Mat4 result;
			to_matrix(result);
			return result;
		}

		inline Mat4			to_view_matrix() const
		{
			Mat4 result;
			to_view_matrix(result);
			return result;
		}

	public: // other functions
		inline Vec3			project_vector(			const Vec3&				VECTOR,
													const uint32_t			AXIS_ID) const
		{
			return axes()[AXIS_ID] * glm::dot(axes()[AXIS_ID], VECTOR);
		}

		/*
			Projects 3D point from global 3D space to local 2D space.
			X_2D_INDEX and Y_2D_INDEX specify indices of the axes in local 2D space.
		*/
		Vec2				project_point(			const Vec3&				POINT_3D,
													const uint32_t			X_2D_INDEX,
													const uint32_t			Y_2D_INDEX) const;

		/*
			Unprojects 2D point from local 2D space back to global 3D space.
			X_2D_INDEX and Y_2D_INDEX specify indices of the axes in local 2D space.
		*/
		Vec3				unproject_point(		const Vec2&				POINT_2D,
													const uint32_t			X_2D_INDEX,
													const uint32_t			Y_2D_INDEX) const;

	private: // functions
		inline void			validate_axis(			const uint32_t			AXIS_INDEX) const
		{
#ifdef _DEBUG
			if(AXIS_INDEX >= 3)
				throw dpl::GeneralException(this, __LINE__, "Unknown axis index: " + std::to_string(AXIS_INDEX));
#endif // _DEBUG
		}
	};
}