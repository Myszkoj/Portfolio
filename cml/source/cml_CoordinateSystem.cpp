#include "..//include/cml_CoordinateSystem.h"
#include "..//include/cml_Plane.h"
#include "..//include/cml_EulerAngles.h"


namespace cml
{
//=====> CoordinateSystem public: // constants
	const Vec3	CoordinateSystem::GLOBAL_ORIGIN(0.f, 0.f, 0.f);

	const Mat3	CoordinateSystem::GLOBAL_AXES(	Vec3(1.f, 0.f, 0.f),
												Vec3(0.f, 1.f, 0.f),
												Vec3(0.f, 0.f, 1.f));

	const Vec3	CoordinateSystem::GLOBAL_FRONT	= GLOBAL_AXES[0];
	const Vec3	CoordinateSystem::GLOBAL_BACK	= -GLOBAL_AXES[0];
	const Vec3	CoordinateSystem::GLOBAL_UP		= GLOBAL_AXES[1];
	const Vec3	CoordinateSystem::GLOBAL_DOWN	= -GLOBAL_AXES[1];
	const Vec3	CoordinateSystem::GLOBAL_RIGHT	= GLOBAL_AXES[2];
	const Vec3	CoordinateSystem::GLOBAL_LEFT	= -GLOBAL_AXES[2];

//=====> CoordinateSystem public: // XYZ axes
	void		CoordinateSystem::reset(					const Mat4&				TRANSFORMATION,
															Vec3&					scale)
	{
		glm::mat4x3& result = (*data);

		for(uint32_t i = 0; i < 3; ++i)
		{
			const Vec3& TMP = TRANSFORMATION[i];
			scale[i]	= calculate_length(TMP);
			result[i]	= TMP / scale[i];
		}

		set_origin(	TRANSFORMATION[3]);
	}

	void		CoordinateSystem::turn_towards(				const glm::vec3&		TARGET)
	{
		const auto	TO_TARGET	= TARGET - origin();
		if(const auto DISTANCE	= glm::length(TO_TARGET))
		{
			cml::EulerAngles	angles;
								angles.set_from_direction(TO_TARGET / DISTANCE);

			reset_axes(angles.to_quat());
		}
	}

//=====> CoordinateSystem public: // to-view transformation
	void		CoordinateSystem::to_matrix(				Mat4&					view) const
	{
		view[0][0] = local_X().x;
		view[0][1] = local_X().y;
		view[0][2] = local_X().z;
		view[0][3] = 0.f;
		//view[3][0] = calculate_dot(	local_X(), origin() );

		view[1][0] = local_Y().x;
		view[1][1] = local_Y().y;
		view[1][2] = local_Y().z;
		view[1][3] = 0.f;
		//view[3][1] = calculate_dot(	local_Y(), origin() );

		view[2][0] = local_Z().x;
		view[2][1] = local_Z().y;
		view[2][2] = local_Z().z;
		view[2][3] = 0.f;	
		//view[3][2] = calculate_dot(	local_Z(), origin() );

		view[3][0] = origin().x;
		view[3][1] = origin().y;
		view[3][2] = origin().z;
		view[3][3] = 1.f;
	}

	void		CoordinateSystem::to_view_matrix(			Mat4&					view) const
	{
		view[0][0] = right().x;
		view[1][0] = right().y;
		view[2][0] = right().z;
		view[3][0] = -calculate_dot(	right(),	origin() );

		view[0][1] = up().x;
		view[1][1] = up().y;
		view[2][1] = up().z;
		view[3][1] = -calculate_dot(	up(),		origin() );

		view[0][2] = -front().x;
		view[1][2] = -front().y;
		view[2][2] = -front().z;
		view[3][2] = calculate_dot(		front(),	origin() );

		view[0][3] = 0.f;
		view[1][3] = 0.f;
		view[2][3] = 0.f;
		view[3][3] = 1.f;

		/*
		view[0][0] = X().x;
		view[1][0] = X().y;
		view[2][0] = X().z;
		view[3][0] = -calculate_dot(	X(),	origin() );

		view[0][1] = Y().x;
		view[1][1] = Y().y;
		view[2][1] = Y().z;
		view[3][1] = -calculate_dot(	Y(),	origin() );

		view[0][2] = Z().x;
		view[1][2] = Z().y;
		view[2][2] = Z().z;
		view[3][2] = -calculate_dot(	Z(),	origin() );

		view[0][3] = 0.f;
		view[1][3] = 0.f;
		view[2][3] = 0.f;
		view[3][3] = 1.f;
		*/

		//const Mat4 ROT = glm::toMat4(glm::angleAxis(glm::radians(90.f), GLOBAL_UP));
		
		//view = ROT * view;

		/*
		// Inverse right distance.
		view[3][0] *= -1.f;

		// Inverse Up distance.
		view[3][1] *= -1.f;

		// Inverse front direction
		view[0][2] *= -1.f;
		view[1][2] *= -1.f;
		view[2][2] *= -1.f;
		*/
	}

//=====> CoordinateSystem public: // other functions
	Vec2		CoordinateSystem::project_point(			const Vec3&				POINT_3D,
															const uint32_t			X_2D_INDEX,
															const uint32_t			Y_2D_INDEX) const
	{
		validate_axis(X_2D_INDEX);
		validate_axis(Y_2D_INDEX);

		const uint32_t	NORMAL_AXIS_INDEX = 3 - X_2D_INDEX - Y_2D_INDEX;
		const Plane		PROJECTION_PLANE(origin(), axes()[NORMAL_AXIS_INDEX]);
		const Vec3		PROJECTED_POINT_3D = PROJECTION_PLANE.project_point(POINT_3D - origin());

		return Vec2(glm::dot(axes()[X_2D_INDEX], PROJECTED_POINT_3D), 
					glm::dot(axes()[Y_2D_INDEX], PROJECTED_POINT_3D));
	}

	Vec3		CoordinateSystem::unproject_point(			const Vec2&				POINT_2D,
															const uint32_t			X_2D_INDEX,
															const uint32_t			Y_2D_INDEX) const
	{
		validate_axis(X_2D_INDEX);
		validate_axis(Y_2D_INDEX);

		return origin() + axes()[X_2D_INDEX] * POINT_2D.x + axes()[Y_2D_INDEX] * POINT_2D.y;
	}
}