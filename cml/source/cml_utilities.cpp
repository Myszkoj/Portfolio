#include "../include/cml_utilities.h"
//#include "../include/Core_Angle.h"

#pragma warning( disable : 26451)

namespace cml
{
	uint32_t				calculate_digits(					uint32_t			x)
	{
		if (x >= 10000) {
			if (x >= 10000000) {
				if (x >= 100000000) {
					if (x >= 1000000000)
						return 10;
					return 9;
				}
				return 8;
			}
			if (x >= 100000) {
				if (x >= 1000000)
					return 7;
				return 6;
			}
			return 5;
		}
		if (x >= 100) {
			if (x >= 1000)
				return 4;
			return 3;
		}
		if (x >= 10)
			return 2;
		return 1;
	}

	glm::quat				look_at(							const Vec3&			eye,
																const Vec3&			center,
																const Vec3&			up,
																const Vec3&			alternativeUp)
	{
		Vec3	direction       = center - eye;
		float   directionLength = glm::length(direction);

		// Check if the direction is valid; Also deals with NaN
		if(!(directionLength > 0.0001f))
			return glm::quat(1.f, 0.f, 0.f, 0.f); // Just return identity

		// Normalize direction
		direction /= directionLength;

		// Is the normal up (nearly) parallel to direction?
		if(glm::abs(glm::dot(direction, up)) > 0.9999f) 
		{
			// Use alternative up
			return glm::quatLookAt(direction, alternativeUp);
		}

		return glm::quatLookAt(direction, up);
	}

	inline float			calculate_dx(						const float			DISTANCE,
																const float			RADIUS,
																const float			SINUS)
	{
		return DISTANCE - RADIUS * SINUS;
	}

	inline float			calculate_dy(						const float			RADIUS,
																const float			SINUS)
	{
		return RADIUS * glm::sqrt(1.f - glm::min(SINUS * SINUS, 1.f));
	}


	Vec2					calculate_left_tangent(				const Vec2&			DIRECTION,
																const float			DISTANCE,
																const float			RADIUS)
	{
		const float SINUS = glm::max(0.f, RADIUS/DISTANCE);
		return calculate_dx(DISTANCE, RADIUS, SINUS) * DIRECTION 
			 + calculate_dy(RADIUS, SINUS) * calculate_left_vector(DIRECTION);
	}

	Vec2					calculate_right_tangent(			const Vec2&			DIRECTION,
																const float			DISTANCE,
																const float			RADIUS)
	{
		const float SINUS = glm::max(0.f, RADIUS/DISTANCE);
		return calculate_dx(DISTANCE, RADIUS, SINUS) * DIRECTION 
			 + calculate_dy(RADIUS, SINUS) * calculate_right_vector(DIRECTION);
	}

	float					signed_distance(					const Vec2&			begin, 
																Vec2				end, 
																Vec2				point)
	{
		end		-= begin;
		point	-= begin;

		const float DOT = glm::dot(end, end);
		return (DOT > 0.f)	? calculate_det(end, point) / glm::sqrt(DOT)
							: std::numeric_limits<float>::quiet_NaN();
	}

	/*
	bool right_side_projection(const Point3D& begin, Point3D end, Point3D point)
	{
		end.x	-= begin.x;
		end.z	-= begin.z;
		point.x -= begin.x;
		point.z -= begin.z;

		return (point.x*end.z - point.z*end.x) > 0.f;
	}

	bool left_side_projection(const Point3D& begin, Point3D end, Point3D point)
	{
		end.x	-= begin.x;
		end.z	-= begin.z;
		point.x -= begin.x;
		point.z -= begin.z;

		return (point.x*end.z - point.z*end.x) < 0.f;
	}
	*/

	bool					lines_intersect(					const Vec2&			p1, 
																const Vec2&			q1, 
																const Vec2&			p2, 
																const Vec2&			q2)
	{
		// SOURCE: https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
		// Find the four orientations needed for general and special cases.
		const Orientation O1 = check_orientation(p1, q1, p2); 
		const Orientation O2 = check_orientation(p1, q1, q2); 
		const Orientation O3 = check_orientation(p2, q2, p1); 
		const Orientation O4 = check_orientation(p2, q2, q1); 
  
		// General case 
		if (O1 != O2 && O3 != O4) 
			return true; 
  
		// Special cases
		if (O1 == Orientation::COLLINEAR && point_on_line_segment(p1, q1, p2)) return true; 
		if (O2 == Orientation::COLLINEAR && point_on_line_segment(p1, q1, q2)) return true; 
		if (O3 == Orientation::COLLINEAR && point_on_line_segment(p2, q2, p1)) return true; 
		if (O4 == Orientation::COLLINEAR && point_on_line_segment(p2, q2, q1)) return true; 
  
		return false; // Doesn't fall in any of the above cases 
	}

	Vec2					intersection_point(					const Vec2&			l1, 
																const Vec2&			l2, 
																const Vec2&			m1, 
																const Vec2&			m2)
	{
		Vec2 a = l1 - l2;
		Vec2 b = m1 - m2;

		float detDiv = a.x * b.y - a.y * b.x;

		float detL = l1.x*l2.y - l1.y*l2.x;
		float detM = m1.x*m2.y - m1.y*m2.x;

		return Vec2((detL*b.x - a.x*detM)/detDiv, (detL*b.y - a.y*detM)/detDiv);
	}
	
	std::optional<float>	signed_Y_distance(					const Vec2&			EDGE_START, 
																const Vec2&			EDGE_END, 
																const Vec2&			POINT)
	{
		if(const float DX = EDGE_END.x - EDGE_START.x)
		{
			const float DY = EDGE_END.y - EDGE_START.y;
			return POINT.y - ((POINT.x - EDGE_START.x)) * DY / DX - EDGE_START.y; 
		}
		
		const float X_DISTANCE = POINT.x - EDGE_START.x;
		if(X_DISTANCE == 0.f)
		{
			const float MIN_Y = glm::min(EDGE_START.y, EDGE_END.y);
			const float MAX_Y = glm::max(EDGE_START.y, EDGE_END.y);

			if(POINT.y < MIN_Y) return POINT.y - MIN_Y;
			if(POINT.y > MAX_Y) return POINT.y - MAX_Y;
			return 0.f;
		}

		return std::nullopt; 
	}

	Orientation				check_orientation(					const Vec2&			a, 
																const Vec2&			b, 
																const Vec2&			c)
	{
		const float VALUE = calculate_det(b-a, c-a);

		if (VALUE < -glm::epsilon<float>())	return Orientation::CW;
		if (VALUE > glm::epsilon<float>())	return Orientation::CCW;

		return Orientation::COLLINEAR;
	}

	template<>
	bool					intersect<LESS>(					const Vec2&			ep1, 
																const Vec2&			ep2, 
																const Vec2&			point)
	{
		return point.x < glm::max(ep1.x, ep2.x)
			&& point.x > glm::min(ep1.x, ep2.x)
			&& point.y < glm::max(ep1.y, ep2.y)
			&& point.y > glm::min(ep1.y, ep2.y);
	}

	template<>
	bool					intersect<LESS_EQUAL>(				const Vec2&			ep1, 
																const Vec2&			ep2, 
																const Vec2&			point)
	{
		return point.x <= glm::max(ep1.x, ep2.x)
			&& point.x >= glm::min(ep1.x, ep2.x)
			&& point.y <= glm::max(ep1.y, ep2.y)
			&& point.y >= glm::min(ep1.y, ep2.y);
	}

	float					calculate_signed_area(				const Vec2*			POLYGON, 
																const uint32_t		NUM_VERTICES)
	{
		float sum = 0.f;

		if(NUM_VERTICES > 1)
		{
			for(uint32_t secondID = 1; secondID < NUM_VERTICES; ++secondID)
			{
				const uint32_t FIRST_ID = secondID-1;

				const Vec2& EDGE_BEGIN	= POLYGON[FIRST_ID];
				const Vec2& EDGE_END	= POLYGON[secondID];

				sum += (EDGE_END.x - EDGE_BEGIN.x) * (EDGE_END.y + EDGE_BEGIN.y);
			}

			const Vec2& EDGE_BEGIN	= POLYGON[NUM_VERTICES-1];
			const Vec2& EDGE_END	= POLYGON[0];

			sum += (EDGE_END.x - EDGE_BEGIN.x) * (EDGE_END.y + EDGE_BEGIN.y);
		}

		return sum / 2.f;
	}

	Orientation				calculate_polygon_orientation(		const float			SIGNED_AREA)
	{
		if(SIGNED_AREA < -glm::epsilon<float>())	return Orientation::CW;
		if(SIGNED_AREA > glm::epsilon<float>())		return Orientation::CCW;

		return Orientation::COLLINEAR;
	}

	bool					is_polygon_degenerated(				const Vec2*			POLYGON, 
																const uint32_t		NUM_VERTICES,
																const float			MIN_EDGE_LENGTH)
	{
		if(NUM_VERTICES < 3)
			return true;

		for(uint32_t firstID = 0; firstID < NUM_VERTICES; ++firstID)
		{
			const uint32_t FIRST_END_ID = (firstID+1)%NUM_VERTICES;

			const Vec2& BEGIN_A	= POLYGON[firstID];
			const Vec2& END_A	= POLYGON[FIRST_END_ID];

			const float A_LENGTH = glm::distance(BEGIN_A, END_A);
			if(A_LENGTH < MIN_EDGE_LENGTH)
			{
				return true;
			}

			for(uint32_t secondID = firstID+1; secondID < NUM_VERTICES; ++secondID)
			{
				const uint32_t SECOND_END_ID = (secondID+1)%NUM_VERTICES;

				const Vec2& BEGIN_B	= POLYGON[secondID];
				const Vec2& END_B	= POLYGON[SECOND_END_ID];

				const bool bFIRST_IS_SECOND_END = (firstID == SECOND_END_ID);
				const bool bSECOND_IS_FIRST_END = (secondID == FIRST_END_ID);

				if(bFIRST_IS_SECOND_END && !bSECOND_IS_FIRST_END)
				{
					const float BEGIN_LENGTH	= glm::distance(BEGIN_A,	BEGIN_B);
					const float END_LENGTH		= glm::distance(END_A,		BEGIN_B);

					if(A_LENGTH >= BEGIN_LENGTH + END_LENGTH)
					{
						return true;
					}
				}
				else if(!bFIRST_IS_SECOND_END && bSECOND_IS_FIRST_END)
				{
					const float BEGIN_LENGTH	= glm::distance(BEGIN_A,	END_B);
					const float END_LENGTH		= glm::distance(END_A,		END_B);

					if(A_LENGTH >= BEGIN_LENGTH + END_LENGTH)
					{
						return true;
					}
				}
				else if(!bFIRST_IS_SECOND_END && !bSECOND_IS_FIRST_END)
				{
					if(lines_intersect(BEGIN_A, END_A, BEGIN_B, END_B))
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	/*
		This algorithm was designed specifically for the point-in-polygon test.
		We assume that point cannot lie on the edge.
	*/
	bool					open_lines_intersect(				const Vec2&			RAY_START, 
																const Vec2&			RAY_END, 
																const Vec2&			EDGE_START, 
																const Vec2&			EDGE_END)
	{
		if(EDGE_START.y == RAY_START.y || EDGE_END.y == RAY_START.y)
		{
			const float Y_LENGTH = glm::abs(EDGE_START.y - EDGE_END.y);
			if(Y_LENGTH > 0.f)
			{
				const Vec2	EDGE_DIRECTION	= normalize_vector(EDGE_END-EDGE_START);
				const float	MAX_DISTANCE	= glm::abs(calculate_det(EDGE_DIRECTION, RAY_START-EDGE_START));
				const float	DELTA_Y			= glm::min(Y_LENGTH, MAX_DISTANCE) / 2.f;

				if(DELTA_Y > 0.f)
				{
					const Vec2 DELTA(0.f, DELTA_Y);
					return open_lines_intersect(RAY_START, RAY_END, EDGE_START+DELTA, EDGE_END+DELTA);
				}
				else
				{
					return false;
				}
			}
			else // Ray and edge are parallel and edge lies on the ray.
			{
				return false;
			}
		}

		const Orientation REs = check_orientation(RAY_START, RAY_END, EDGE_START);
		const Orientation REe = check_orientation(RAY_START, RAY_END, EDGE_END);

		if(REs == REe)
			return false;
	
		const Orientation ERs = check_orientation(EDGE_START, EDGE_END, RAY_START);
		if(ERs == Orientation::COLLINEAR)
			return false;
		
		const Orientation ERe = check_orientation(EDGE_START, EDGE_END, RAY_END);
		if(ERe == Orientation::COLLINEAR)
			return false;

		if(ERs == ERe)
			return false;

		return true; // General case.
	}

	bool					point_in_polygon(					const Vec2*			POLYGON, 
																const uint32_t		NUM_VERTICES,
																const Vec2&			POINT)
	{
		// SOURCE: https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/

		if (NUM_VERTICES < 3) return false; // It's not a polygon.
 
		// This value will be added to the ray to assure that it crosses the edge.
		const float SURPLUS = 100.f;
 
		// Count intersections of the above line with sides of polygon
		uint32_t count = 0, currentID = 0;

		do
		{
			const uint32_t	NEXT_ID = (currentID+1)%NUM_VERTICES;
			const Vec2&		CURRENT = POLYGON[currentID];
			const Vec2&		NEXT	= POLYGON[NEXT_ID];

			float deltaX = glm::abs(CURRENT.x - POINT.x) + glm::abs(NEXT.x - POINT.x) + SURPLUS;

			const Vec2		EXTREME = {POINT.x + deltaX, POINT.y};

			if(open_lines_intersect(POINT, EXTREME, CURRENT, NEXT))
			{
				++count;
			}

			currentID = NEXT_ID;
		} while (currentID != 0);

		// Return true if count is odd, false otherwise
		return count&1; // Same as (count%2 == 1)
	}

	Vec3					calculate_barycentric_coordinates(	const Vec3&			A,
																const Vec3&			B,
																const Vec3&			C,
																const Vec3&			P) 
	{
		Vec3 v0 = B - A; 
		Vec3 v1 = C - A;
		Vec3 v2 = P - A;

		// this can be cached in triangle structure
		float d00 = glm::dot(v0, v0);
		float d01 = glm::dot(v0, v1);
		float d11 = glm::dot(v1, v1);
		float denom = d00 * d11 - d01 * d01; // (2*area)^2

		float d20 = glm::dot(v2, v0);
		float d21 = glm::dot(v2, v1);

		float v = (d11 * d20 - d01 * d21) / denom;
		float w = (d00 * d21 - d01 * d20) / denom;
		
		return Vec3(v, w, 1.f - v - w);
	}

	/*
	float			calculate_yaw(						const Quat&			ROT)
	{
		const float TOP_VALUE		= 2.f * (ROT.w * ROT.y - ROT.x * ROT.z);
		const float BOTTOM_VALUE	= ROT.w * ROT.w - ROT.x * ROT.x - ROT.y * ROT.y + ROT.z * ROT.z;

		if((TOP_VALUE == 0.f) && (BOTTOM_VALUE == 0.f))
			return 2.f * std::atan2(-ROT.y, -ROT.w);

		return std::atan2(TOP_VALUE, BOTTOM_VALUE);
	}

	float			calculate_pitch(					const Quat&			ROT)
	{
		return std::asin(glm::clamp(2.f * (ROT.w * ROT.x + ROT.y * ROT.z), -1.f, 1.f));
	}

	float			calculate_roll(						const Quat&			ROT)
	{
		const float TOP_VALUE		= 2.f * (ROT.x * ROT.y - ROT.w * ROT.z);
		const float BOTTOM_VALUE	= ROT.w * ROT.w - ROT.x * ROT.x + ROT.y * ROT.y - ROT.z * ROT.z;

		return std::atan2(TOP_VALUE, BOTTOM_VALUE);
	}
	*/

	void					translate_matrix(					const Vec3&			v,
																Mat4&				mat)
	{
		mat[3] = mat[0] * v[0] + mat[1] * v[1] + mat[2] * v[2] + mat[3];
	}

	void					scale_matrix(						const Vec3&			v,
																Mat4&				mat)
	{
		mat[0] = mat[0] * v[0];
		mat[1] = mat[1] * v[1];
		mat[2] = mat[2] * v[2];
	}

	glm::mat4				toMat4(								const Quat&			q)
	{
		const float qxx(q.x * q.x);
		const float qyy(q.y * q.y);
		const float qzz(q.z * q.z);
		const float qxz(q.x * q.z);
		const float qxy(q.x * q.y);
		const float qyz(q.y * q.z);
		const float qwx(q.w * q.x);
		const float qwy(q.w * q.y);
		const float qwz(q.w * q.z);

		return glm::mat4(
			1.f - 2.f * (qyy +  qzz),		2.f * (qxy + qwz),			2.f * (qxz - qwy),			0.f,
			2 * (qxy - qwz),				1 - 2 * (qxx +  qzz),		2 * (qyz + qwx),			0.f,
			2 * (qxz + qwy),				2 * (qyz - qwx),			1 - 2 * (qxx +  qyy),		0.f,
			0.f,							0.f,						0.f,						1.f
		);
	}

	void					set_TRS_matrix(						const Vec3&			T, 
																const Quat&			R, 
																const Vec3&			S,
																Mat4&				mat)
	{
		const float qxx(R.x * R.x);
		const float qyy(R.y * R.y);
		const float qzz(R.z * R.z);
		const float qxz(R.x * R.z);
		const float qxy(R.x * R.y);
		const float qyz(R.y * R.z);
		const float qwx(R.w * R.x);
		const float qwy(R.w * R.y);
		const float qwz(R.w * R.z);

		mat[0][0] = (1.f - 2.f * (qyy +  qzz))	* S[0];
		mat[0][1] = 2.f * (qxy + qwz)			* S[0];
		mat[0][2] = (2.f * (qxz - qwy))			* S[0];
		mat[0][3] = 0.f;
	
		mat[1][0] = (2.f * (qxy - qwz))			* S[1];	
		mat[1][1] = (1.f - 2.f * (qxx +  qzz))	* S[1];
		mat[1][2] = (2.f * (qyz + qwx))			* S[1];
		mat[1][3] = 0.f;

		mat[2][0] = (2.f * (qxz + qwy))			* S[2];
		mat[2][1] = (2.f * (qyz - qwx))			* S[2];
		mat[2][2] = (1.f - 2.f * (qxx +  qyy))	* S[2];
		mat[2][3] = 0.f;

		mat[3][0] = T[0];
		mat[3][1] = T[1];
		mat[3][2] = T[2];
		mat[3][3] = 1.f;
	}

	float					angle_between_0_and_360(			Vec2				ref, 
																Vec2				V)
	{
		const float dotU = glm::sqrt(glm::dot(ref, ref));
		if(dotU == 0.f) return 0.f;

		float dotV = glm::sqrt(glm::dot(V, V));
		if(dotV == 0.f) return 0.f;

		// normalize
		ref = ref/dotU;
		V = V/dotV;

		return wrap_angle360(atan2(calculate_det(ref, V), calculate_dot(ref, V)));
	}
}