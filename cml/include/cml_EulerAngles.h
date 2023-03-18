#pragma once


#include <dpl_ReadOnly.h>
#include "cml_CoordinateSystem.h"


namespace cml
{
	/*
		Stores roll, yaw, pitch angles in radians.
		Rotations are applied in the following order: Yaw -> Pitch -> Roll
	*/
	class EulerAngles
	{
	public: // constants
		static const float HALF_PI;
		static const float MAX_HALF_SIN_PITCH;

	public: // data
		dpl::ReadOnly<float, EulerAngles> yaw;	// Rotation around local +Y axis. Range: <-PI, +PI>
		dpl::ReadOnly<float, EulerAngles> pitch; // Rotation around local +Z axis. Range: <-PI/2, +PI/2>
		dpl::ReadOnly<float, EulerAngles> roll;	// Rotation around local +X axis. Range: <-PI, +PI>

	public: // lifecycle
		CLASS_CTOR		EulerAngles()
			: yaw(0.f)
			, pitch(0.f)
			, roll(0.f)
		{

		}

		CLASS_CTOR		EulerAngles(		const float			YAW_RADIANS,
											const float			PITCH_RADIANS,
											const float			ROLL_RADIANS)
			: yaw(wrap_angle180(YAW_RADIANS))
			, pitch(glm::clamp(PITCH_RADIANS, -HALF_PI, HALF_PI))
			, roll(wrap_angle180(ROLL_RADIANS))
		{

		}

		CLASS_CTOR		EulerAngles(		const Vec3&			YPR_RADIANS)
			: EulerAngles(YPR_RADIANS.x, YPR_RADIANS.y, YPR_RADIANS.z)
		{

		}

		CLASS_CTOR		EulerAngles(		const Quat&			ROTATION)
			: EulerAngles()
		{
			set_from_quat(ROTATION);
		}

		CLASS_CTOR		EulerAngles(		const glm::vec3&	NORMALIZED_FRONT,
											const glm::vec3&	NORMALIZED_UP)
			: EulerAngles()
		{
			set_from_direction(NORMALIZED_FRONT);
			set_roll(glm::orientedAngle(CoordinateSystem::GLOBAL_UP, NORMALIZED_UP, NORMALIZED_FRONT));
		}

	public: // functions
		inline void		set_yaw(			const float			YAW_RADIANS)
		{
			yaw = wrap_angle180(YAW_RADIANS);
		}

		inline void		set_pitch(			const float			PITCH_RADIANS)
		{
			pitch = glm::clamp(PITCH_RADIANS, -HALF_PI, HALF_PI);
		}

		inline void		set_roll(			const float			ROLL_RADIANS)
		{
			roll = wrap_angle180(ROLL_RADIANS);
		}

		inline void		set(				const float			YAW_RADIANS,
											const float			PITCH_RADIANS,
											const float			ROLL_RADIANS)
		{
			set_yaw(YAW_RADIANS);
			set_pitch(PITCH_RADIANS);
			set_roll(ROLL_RADIANS);
		}

		void			set_from_direction(	const Vec3&			NORMALIZED_LOCAL_FRONT);

		/*
			Sets yaw, pitch, roll from given quaternion.
		*/
		void			set_from_quat(		const Quat&			ROTATION);

		/*
			Extracts only yaw angle from given quaternion.
			Note that setting individual angles is slower.
		*/
		void			set_only_yaw(		const Quat&			ROTATION);

		/*
			Extracts only pitch angle from given quaternion.
			Note that setting individual angles is slower.
		*/
		void			set_only_pitch(		const Quat&			ROTATION);

		/*
			Extracts only roll angle from given quaternion.
			Note that setting individual angles is slower.
		*/
		void			set_only_roll(		const Quat&			ROTATION);

		/*
			Converts yaw -> pitch -> roll to quaternion.
		*/
		Quat			to_quat() const;

		/*
			Packs yaw -> pitch -> roll to xyz respectively.
		*/
		inline Vec3		to_vector() const
		{
			return Vec3(yaw(), pitch(), roll());
		}

	private: // functions
		/*
			Set yaw angle from vector projected on the XZ plane.
		*/
		inline void		set_yaw_internal(	const Vec3&			NORMALIZED_LOCAL_FRONT)
		{
			const Vec2 XZ_PROJECTION = normalize_vector(Vec2(NORMALIZED_LOCAL_FRONT.x, NORMALIZED_LOCAL_FRONT.z));
			yaw = wrap_angle180(std::atan2(-XZ_PROJECTION.y, XZ_PROJECTION.x));
		}

		inline void		set_pitch_internal(	const Vec3&			NORMALIZED_LOCAL_FRONT)
		{
			set_pitch(ANGLE_90 - glm::angle(CoordinateSystem::GLOBAL_UP, NORMALIZED_LOCAL_FRONT));
		}

		static void		test_YPR_quat_conversions();
	};
}