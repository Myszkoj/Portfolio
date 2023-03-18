#include "..//include/cml_Ray.h"
#include "..//include/cml_Plane.h"


namespace cml
{
//=====> Ray public: // functions
	void					Ray::set(					const Vec2&		TARGET,
														const Vec2&		VIEW_MIN,
														const Vec2&		VIEW_MAX,
														const Mat4&		VIEW_PROJECTION)
	{
		const Mat4    INV_VIEW_PROJECTION = glm::inverse(VIEW_PROJECTION);

		const float WIDTH   = VIEW_MAX.x - VIEW_MIN.x;
		const float HEIGHT  = VIEW_MAX.y - VIEW_MIN.y;

		const float NORMALIZED_MOUSE_X = ((TARGET.x - VIEW_MIN.x) / WIDTH) * 2.f - 1.f;
		const float NORMALIZED_MOUSE_Y = (1.f - ((TARGET.y - VIEW_MIN.y) / HEIGHT)) * 2.f - 1.f;

		const Vec4 BEGIN	= INV_VIEW_PROJECTION * Vec4(NORMALIZED_MOUSE_X, NORMALIZED_MOUSE_Y, 0.f, 1.f);
		const Vec4 END		= INV_VIEW_PROJECTION * Vec4(NORMALIZED_MOUSE_X, NORMALIZED_MOUSE_Y, 1.f, 1.f);

		*origin		= BEGIN / BEGIN.w;
		*direction	= normalize_vector(Vec3(END / END.w) - origin());
	}

	std::optional<float>	Ray::calculate_distance(	const Plane&	PLANE) const
	{
		// source: https://stackoverflow.com/questions/23975555/how-to-do-ray-plane-intersection

		const float DISTANCE_FACTOR = -glm::dot(PLANE.normal(), direction());

        if (fabsf(DISTANCE_FACTOR) < FLT_EPSILON)  // normal is orthogonal to vector, cant intersect
        {
            return std::nullopt;
        }

        const float DISTANCE_ALONG_PLANE_NORMAL = glm::dot(PLANE.normal(), origin()) - PLANE.distance();
        return DISTANCE_ALONG_PLANE_NORMAL / DISTANCE_FACTOR;
	}
}