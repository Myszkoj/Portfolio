#pragma once


#define GLM_FORCE_PURE
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/matrix_interpolation.hpp"
#include <limits>
#include <optional>
#include <dpl_ClassInfo.h>


namespace cml // core-math-lib
{
	using Vec2	= glm::vec2;
	using IVec2 = glm::tvec2<int32_t>;
	using UVec2 = glm::tvec2<uint32_t>;

	using Vec3	= glm::vec3;
	using IVec3 = glm::tvec3<int32_t>;
	using UVec3 = glm::tvec3<uint32_t>;

	using Vec4	= glm::vec4;
	using IVec4 = glm::tvec4<int32_t>;
	using UVec4 = glm::tvec4<uint32_t>;

	using Quat = glm::quat;

	using Mat2 = glm::mat2;
	using Mat3 = glm::mat3;
	using Mat4 = glm::mat4;

	const float FLOAT_INFINITY	= std::numeric_limits<float>::max();
	const Quat	QUAT_ZERO(1.f, 0.f, 0.f, 0.f);
	const Mat3	MAT3_IDENTITY(1.f);
	const Mat4	MAT4_IDENTITY(1.f);
	const float ANGLE_90	= glm::pi<float>()/2.f;
	const float ANGLE_180	= glm::pi<float>();
	const float ANGLE_270	= glm::pi<float>()*(3.f/2.f);
	const float ANGLE_360	= glm::pi<float>()*2.f;
	const float PLUS_EPSILON	= 0.0001f;
	const float MINUS_EPSILON	= -0.0001f;


	template<bool bSigned>
	class NormalizedFloat
	{
	public: // constants
		static constexpr bool	SIGNED	= bSigned;
		static constexpr float	MIN		= SIGNED ? -1.f : 0.f;
		static constexpr float	MAX		= 1.f;

	private: // data
		float m_value;

	public: // lifecycle
		CLASS_CTOR				NormalizedFloat()
			: m_value(0.f)
		{

		}

		template<typename U>
		CLASS_CTOR				NormalizedFloat(	const U		OTHER_VALUE)
			: m_value(glm::clamp((float)OTHER_VALUE, MIN, MAX))
		{

		}

	public: // operators
		inline operator			const float() const
		{
			return m_value;
		}

		template<typename U>
		inline NormalizedFloat& operator=(			const U&	OTHER_VALUE)
		{
			m_value = glm::clamp(OTHER_VALUE, MIN, MAX);
			return *this;
		}

		template<typename U>
		inline bool				operator==(			const U		OTHER_VALUE) const
		{
			return m_value == OTHER_VALUE;
		}

		template<typename U>
		inline bool				operator!=(			const U		OTHER_VALUE) const
		{
			return m_value != OTHER_VALUE;
		}

		template<typename U>
		inline NormalizedFloat& operator+=(			const U		OTHER_VALUE)
		{
			m_value = glm::clamp(m_value + OTHER_VALUE, MIN, MAX);
			return *this;
		}

		template<typename U>
		inline NormalizedFloat& operator-=(			const U		OTHER_VALUE)
		{
			m_value = glm::clamp(m_value - OTHER_VALUE, MIN, MAX);
			return *this;
		}

		template<typename U>
		inline NormalizedFloat& operator*=(			const U		OTHER_VALUE)
		{
			m_value = glm::clamp(m_value * OTHER_VALUE, MIN, MAX);
			return *this;
		}

		template<typename U>
		inline NormalizedFloat& operator/=(			const U		OTHER_VALUE)
		{
			m_value = glm::clamp(m_value / OTHER_VALUE, MIN, MAX);
			return *this;
		}
	};


	using NSFloat = NormalizedFloat<true>;
	using NUFloat = NormalizedFloat<false>;

	enum RotationOrder
	{
		XYZ,
		XZY,
		YXZ,
		YZX,
		ZXY,
		ZYX
	};

	enum class Orientation
	{
		COLLINEAR	= 0, // NOTE: Some algorithms are based on this value. Do not change!
		CW			= 1,
		CCW			= 2
	};

	enum CompareOperation
	{
		LESS,
		LESS_EQUAL,
		EQUAL,
		NOT_EQUAL,
		GREATER,
		GREATER_EQUAL
	};

	enum class Side : uint8_t
	{
		eNONE	= 0,
		eRIGHT	= 1,
		eLEFT	= 2
	};

	template<uint32_t L, typename T>
	inline bool				is_nan(									const glm::vec<L, T>& VECTOR)
	{
		for(uint32_t index = 0; index < L; ++index)
		{
			if(glm::isnan(VECTOR[index])) return true;
		}

		return false;
	}

	template<uint32_t L, typename T>
	inline bool				is_infinite(							const glm::vec<L, T>& VECTOR)
	{
		for(uint32_t index = 0; index < L; ++index)
		{
			if(glm::isinf(VECTOR[index])) return true;
		}

		return false;
	}

	template<uint32_t L, typename T>
	inline bool				is_degenerate(							const glm::vec<L, T>& VECTOR)
	{
		for(uint32_t index = 0; index < L; ++index)
		{
			if(glm::isnan(VECTOR[index]) || glm::isinf(VECTOR[index])) return true;
		}

		return false;
	}

	template<uint32_t C, uint32_t R, typename T>
	inline bool				is_nan(									const glm::mat<C, R, T>& MATRIX)
	{
		for(uint32_t column = 0; column < C; ++column)
		{
			if(is_nan<R, T>(MATRIX[column])) return true;
		}

		return false;
	}

	template<uint32_t C, uint32_t R, typename T>
	inline bool				is_infinite(							const glm::mat<C, R, T>& MATRIX)
	{
		for(uint32_t column = 0; column < C; ++column)
		{
			if(is_infinite<R, T>(MATRIX[column])) return true;
		}

		return false;
	}

	template<uint32_t C, uint32_t R, typename T>
	inline bool				is_degenerate(							const glm::mat<C, R, T>& MATRIX)
	{
		for(uint32_t column = 0; column < C; ++column)
		{
			if(is_degenerate<R, T>(MATRIX[column])) return true;
		}

		return false;
	}

	uint32_t				calculate_digits(						uint32_t			x);


	/*
		Returns quaternion that rotates old axis system to the new one.
		Vectors must be perpendicular and normalized.
	*/
	inline Quat				convert_axis_system(					const Vec3&			oldR,
																	const Vec3&			oldU,
																	const Vec3&			oldF,
																	const Vec3&			newR,
																	const Vec3&			newU,
																	const Vec3&			newF)
	{
		return glm::quat_cast(glm::mat3(newR, newU, newF) * glm::inverse(glm::mat3(oldR, oldU, oldF)));
	}

	Quat					look_at(								const Vec3&			eye,
																	const Vec3&			center,
																	const Vec3&			up,
																	const Vec3&			alternativeUp);

	inline Side				inverse(								Side				side)
	{
		return (side == Side::eRIGHT) ? Side::eLEFT : Side::eRIGHT;
	}

	inline float			calculate_quadratic_det(				float				A,
																	float				B,
																	float				C)
	{
		return B*B - 4.f*A*C;
	}

	inline float			calculate_quadratic_root(				float				A,
																	float				B,
																	float				Dsqrt)
	{
		return (-B + Dsqrt)/(2.f*A);
	}

	inline float			calculate_dot(							const Vec2&			v1,
																	const Vec2&			v2)
	{
		return (v1.x*v2.x) + (v1.y*v2.y);
	}

	inline float			calculate_dot(							const Vec3&			v1,
																	const Vec3&			v2)
	{
		return (v1.x*v2.x) + (v1.y*v2.y) + (v1.z*v2.z);
	}

	inline float			calculate_dot(							const Quat&			q1,
																	const Quat&			q2)
	{
		return (q1.w*q2.w) + (q1.x*q2.x) + (q1.y*q2.y) + (q1.z*q2.z);
	}

	inline float			calculate_det(							const Vec2&			a, 
																	const Vec2&			b)
	{
		return a.x*b.y - b.x*a.y;
	}

	inline Vec3				calculate_cross(						const Vec3&			v1,
																	const Vec3&			v2)
	{
		return glm::cross(v1, v2);
	}

	inline float			calculate_length(						const Vec2&			v)
	{
		return sqrt(calculate_dot(v, v));
	}

	inline float			calculate_length(						const Vec3&			v)
	{
		return sqrt(calculate_dot(v, v));
	}

	/*
		Normalizes V if its length is greater than 0.
	*/
	template<class VectorT>
	inline VectorT			normalize_vector(						const VectorT&		v)
	{
		float len = glm::length(v);
		return len > 0.f ? v/len : v;
	}

	inline Quat				normalize_quaternion(					const Quat&			q)
	{
		float len = glm::sqrt(dot(q, q));
		return len <= 0.f ? glm::quat() : q / len;
	}

	inline void				normalize_vector2(						Vec3&				v)
	{
		float len = glm::sqrt(dot(v, v));
		if(len <= 0.f)
		{
			v.x = 0.f;
			v.y = 0.f;
			v.z = 0.f;
		}
		else
		{
			v /= len;
		}
	}

	inline void				normalize_quaternion2(					Quat&				q)
	{
		float len = glm::sqrt(dot(q, q));
		if(len <= 0.f) // Problem
		{
			q = QUAT_ZERO;
		}
		else
		{
			q /= len;
		}
	}

	inline Vec3				calculate_normal(						const Vec3&			POINT_A, 
																	const Vec3&			POINT_B,
																	const Vec3&			POINT_C)
	{
		return normalize_vector(calculate_cross(POINT_B-POINT_A, POINT_C-POINT_A));
	}

	/*
		Calculate parameter t of the point P projected on vector V.
	*/
	template<typename T>
	float					calculate_projection_parameter(			const T&			V, 
																	const T&			P)
	{
		const float SEGMENT_LENGTH = glm::length2(V); 
		return (SEGMENT_LENGTH > 0.f) ? glm::dot(P, V) / SEGMENT_LENGTH : 0.f;
	}

	template<typename T>
	T						project_point_on_line(					const T&			begin, 
																	T					end, 
																	T					point)
	{
		// move origin to begin
		end		-= begin;
		point	-= begin;

		const float t = calculate_projection_parameter(end, point); 

		// begin == end case
		if (t == 0.f) 
			return begin;

		return begin + t * end;  // Projection falls on the segment
	}

	template<typename T>
	T						project_point_on_line_segment(			const T&			begin, 
																	T					end, 
																	T					point)
	{
		// move origin to begin
		end		-= begin;
		point	-= begin;

		const float t = glm::clamp(calculate_projection_parameter(end, point), 0.f, 1.f);

		return begin + t * end;  // Projection falls on the segment
	}

	template<typename T>
	float					distance_to_line(						const T&			begin, 
																	const T&			end, 
																	const T&			point)
	{
		return glm::distance( project_point_on_line(begin, end, point), point );
	}

	template<typename T>
	float					distance_to_line_segment(				const T&			begin, 
																	const T&			end, 
																	const T&			point)
	{
		return glm::distance( project_point_on_line_segment<T>(begin, end, point), point );
	}

	template<typename T>
	bool					point_on_line_segment(					const T&			begin, 
																	const T&			end, 
																	const T&			point)
	{
		static const float EPSILON = 0.00001f;
		return distance_to_line_segment<T>(begin, end, point) < EPSILON;
	}

	/*
		Calculate vector pointing to the right, relative to V.
		Length of the new vector is the same as V's.
	*/
	inline Vec2				calculate_right_vector(					const Vec2&			V)
	{
		return Vec2(V.y, -V.x);
	}

	/*
		Calculate vector pointing to the left, relative to V.
		Length of the new vector is the same as V's.
	*/
	inline Vec2				calculate_left_vector(					const Vec2&			V)
	{
		return Vec2(-V.y, V.x);
	}

	inline Vec2				calculate_side_vector(					const Side			SIDE,
																	const Vec2&			VECTOR)
	{
		return (SIDE == Side::eLEFT) ? calculate_left_vector(VECTOR) : calculate_right_vector(VECTOR);
	}

	/*
		The value of the 'd' must be either 1 or -1.
		If d has a value of 1 vector is rotated 90 degrees to the left,
		if d has a value of -1 vector is rotated 90 degrees to the right.
		Any other value rotates and scales the vector by unknown amount.
	*/
	inline Vec2				rotate_vector(							const Vec2&			V,
																	int32_t				d)
	{
		return Vec2(-static_cast<float>(d) * V.y, 
					static_cast<float>(d) * V.x);
	}

	inline Vec3				Rodrigues_rotation(						const Vec3&			V,
																	const Vec3&			axis,
																	float				radians)
	{
		float X = glm::cos(radians);
		float Y = glm::sin(radians);
		return V*X + glm::cross(axis, V)*Y + axis*glm::dot(axis, V)*(1.f - X);
	}

	inline bool				vequal(									const Vec2&			a, 
																	const Vec2&			b)
	{
		static const float eq = 0.001f*0.001f;

		return glm::distance2(a, b) < eq;
	}

	inline Side				vector_side(							const Vec2&			ref, 
																	const Vec2&			V)
	{
		float det = calculate_det(ref, V);

		if(det > glm::epsilon<float>())
		{
			return Side::eLEFT;
		}
		if(det < -glm::epsilon<float>())
		{
			return Side::eRIGHT;
		}

		return Side::eNONE;
	}

	inline bool				left_side(								const Vec2&			ref, 
																	const Vec2&			V)
	{
		return calculate_det(ref, V) > glm::epsilon<float>();
	}

	inline bool				left_side(								const Vec2&			begin, 
																	const Vec2&			end, 
																	const Vec2&			point)
	{
		return calculate_det(end-begin, point-begin) > glm::epsilon<float>();
	}

	inline bool				left_side_equal(						const Vec2&			ref, 
																	const Vec2&			V)
	{
		return calculate_det(ref, V) >= 0.f;
	}

	inline bool				left_side_equal(						const Vec2&			begin, 
																	const Vec2&			end, 
																	const Vec2&			point)
	{
		return calculate_det(end-begin, point-begin) >= 0.f;
	}

	inline bool				right_side(								const Vec2&			ref, 
																	const Vec2&			V)
	{
		return calculate_det(ref, V) < -glm::epsilon<float>();
	}

	inline bool				right_side(								const Vec2&			begin, 
																	const Vec2&			end, 
																	const Vec2&			point)
	{
		return calculate_det(end-begin, point-begin) < -glm::epsilon<float>();
	}

	inline bool				right_side_equal(						const Vec2&			ref, 
																	const Vec2&			V)
	{
		return calculate_det(ref, V) <= 0.f;
	}

	inline bool				right_side_equal(						const Vec2&			begin, 
																	const Vec2&			end, 
																	const Vec2&			point)
	{
		return calculate_det(end-begin, point-begin) <= 0.f;
	}

	/*
		This functions checks if vector V is between vectors A and B.
	*/
	inline bool				vector_between(							const Vec2&			V,
																	const Vec2&			A,
																	const Vec2&			B)
	{
		const float AxB = calculate_det(A, B);
		const float AxV = calculate_det(A, V);
		const float BxV = calculate_det(B, V);

		return (AxB * AxV >= 0.f) && (AxB * BxV <= 0.f);
	}

	/*
		Calculates vector tangent to the sphere at a given distance from the origin and with given radius.
		Note that direction must be normalized and distance and radius must both be greater than 0.
	*/
	Vec2					calculate_left_tangent(					const Vec2&			DIRECTION,
																	const float			DISTANCE,
																	const float			RADIUS);

	/*
		Calculates vector tangent to the sphere at a given distance from the origin and with given radius.
		Note that direction must be normalized and distance and radius must both be greater than 0.
	*/
	Vec2					calculate_right_tangent(				const Vec2&			DIRECTION,
																	const float			DISTANCE,
																	const float			RADIUS);

	/*
		Calculates vector tangent to the circle at a given distance from the origin and with given radius.
		Note that direction must be normalized and distance and radius must both be greater than 0.
		Returns DIRECTION if Side is eNONE.
	*/
	inline Vec2				calculate_tangent(						const Vec2&			DIRECTION,
																	const float			DISTANCE,
																	const float			RADIUS,
																	const Side			SIDE)
	{
		switch(SIDE)
		{
		case Side::eRIGHT:	return calculate_right_tangent(DIRECTION, DISTANCE, RADIUS);
		case Side::eLEFT:	return calculate_left_tangent(DIRECTION, DISTANCE, RADIUS);
		default:			return DIRECTION;
		}
	}

	/*
		Tests if line p1q1 intersects line p2q2. 
	*/
	bool					lines_intersect(						const Vec2&			p1, 
																	const Vec2&			q1, 
																	const Vec2&			p2, 
																	const Vec2&			q2);

	Vec2					intersection_point(						const Vec2&			l1, 
																	const Vec2&			l2, 
																	const Vec2&			m1, 
																	const Vec2&			m2);

	inline float			signed_triangle_area(					const Vec2&			A, 
																	const Vec2&			B, 
																	const Vec2&			C)
	{
		return calculate_det(B-A, A-C) / 2.f;
	}

	float					signed_distance(						const Vec2&			begin, 
																	Vec2				end, 
																	Vec2				point);



	/*
		TODO: check this equation.
	*/
	inline float			signed_distance(					const Vec3&			ref, 
																const Vec3&			point)
	{
		// ???
		return glm::dot(point, glm::normalize(ref)) + glm::length(ref);
	}

	std::optional<float>	signed_Y_distance(					const Vec2&			EDGE_START, 
																const Vec2&			EDGE_END, 
																const Vec2&			POINT);

	/*
		Check orientation of the triangle abc.
	*/
	Orientation				check_orientation(					const Vec2&			a, 
																const Vec2&			b, 
																const Vec2&			c);

	/*
		Check if point is intersecting bounds created by two end points ep1 and ep2.
	*/
	template<CompareOperation op>
	bool					intersect(								const Vec2&			ep1, 
																	const Vec2&			ep2, 
																	const Vec2&			point);

	template<>
	bool					intersect<LESS>(						const Vec2&			ep1, 
																	const Vec2&			ep2, 
																	const Vec2&			point);

	template<>
	bool					intersect<LESS_EQUAL>(					const Vec2&			ep1, 
																	const Vec2&			ep2, 
																	const Vec2&			point);

	/*
		Check if line l intersects with line m.
	*/
	template<CompareOperation op>
	bool					intersect(								const Vec2&			l1, 
																	const Vec2&			l2, 
																	const Vec2&			m1, 
																	const Vec2&			m2)
	{
		Orientation o1 = check_orientation(l1, l2, m1);
		Orientation o2 = check_orientation(l1, l2, m2);
		Orientation o3 = check_orientation(m1, m2, l1);
		Orientation o4 = check_orientation(m1, m2, l2);

		if (o1 != o2 && o3 != o4)
			return true;

		// special cases
		if (o1 == Orientation::COLLINEAR && intersect<op>(l1, l2, m1)) return true;
		if (o2 == Orientation::COLLINEAR && intersect<op>(l1, l2, m2)) return true;
		if (o3 == Orientation::COLLINEAR && intersect<op>(m1, m2, l1)) return true;
		if (o4 == Orientation::COLLINEAR && intersect<op>(m1, m2, l2)) return true;

		return false;
	}

	/*
		Check if x is between a and b.

		Set op parameter to LESS to get exclusive result,
		or LESS_EQUAL to get inclusive result.
	*/
	template<CompareOperation op>
	inline bool				point_between(							float				x, 
																	float				a, 
																	float				b);

	template<>
	inline bool				point_between<LESS>(					float				x, 
																	float				a, 
																	float				b)
	{
		return (x < a) != (x < b);
	}

	template<>
	inline bool				point_between<LESS_EQUAL>(				float				x, 
																	float				a, 
																	float				b)
	{
		return (x <= a) != (x <= b);
	}



	template<CompareOperation op>
	bool					check_point_with_line(					float				x, 
																	float				y, 
																	float				a, 
																	float				b);

	template<>
	inline bool				check_point_with_line<LESS>(			float				x, 
																	float				y, 
																	float				a, 
																	float				b)
	{
		return y < x*a + b;
	}

	template<>
	inline bool				check_point_with_line<LESS_EQUAL>(		float				x, 
																	float				y, 
																	float				a, 
																	float				b)
	{
		return y <= x*a + b;
	}

	template<>
	inline bool				check_point_with_line<GREATER>(			float				x, 
																	float				y, 
																	float				a, 
																	float				b)
	{
		return y > x*a + b;
	}

	template<>
	inline bool				check_point_with_line<GREATER_EQUAL>(	float				x, 
																	float				y, 
																	float				a, 
																	float				b)
	{
		return y >= x*a + b;
	}

	template<>
	inline bool				check_point_with_line<EQUAL>(			float				x, 
																	float				y, 
																	float				a, 
																	float				b)
	{
		return y == x*a + b;
	}

	template<>
	inline bool				check_point_with_line<NOT_EQUAL>(		float				x, 
																	float				y, 
																	float				a, 
																	float				b)
	{
		return y != x*a + b;
	}

	float					calculate_signed_area(					const Vec2*			POLYGON, 
																	const uint32_t		NUM_VERTICES);

	Orientation				calculate_polygon_orientation(			const float			SIGNED_AREA);

	inline Orientation		calculate_polygon_orientation(			const Vec2*			POLYGON, 
																	const uint32_t		NUM_VERTICES)
	{
		return calculate_polygon_orientation(calculate_signed_area(POLYGON, NUM_VERTICES));
	}

	bool					is_polygon_degenerated(					const Vec2*			POLYGON, 
																	const uint32_t		NUM_VERTICES,
																	const float			MIN_EDGE_LENGTH);

	bool					point_in_polygon(						const Vec2*			POLYGON, 
																	const uint32_t		NUM_VERTICES,
																	const Vec2&			POINT);

	inline float			calculate_distance(						const Vec2&			A,
																	const Vec2&			B)
	{
		return glm::distance(A, B);
	}

	inline float			calculate_distance(						const Vec3&			A,
																	const Vec3&			B)
	{
		return glm::distance(A, B);
	}

	Vec3					calculate_barycentric_coordinates(		const Vec3&			A,
																	const Vec3&			B,
																	const Vec3&			C,
																	const Vec3&			P);

	void					translate_matrix(						const Vec3&			v,
																	Mat4&				mat);

	void					scale_matrix(							const Vec3&			v,
																	Mat4&				mat);

	glm::mat4				toMat4(									const Quat&			q);

	void					set_TRS_matrix(							const Vec3&			T, 
																	const Quat&			R, 
																	const Vec3&			S,
																	Mat4&				mat);

	template<typename T>
	inline T				mix(									const T&			a, 
																	const T&			b, 
																	const float			factor)
	{
		return glm::mix(a, b, factor);
	}

	inline float			wrap_angle360(							float				radians)
	{
		radians = fmod(radians, ANGLE_360);
		if(radians < 0.f)	return radians + ANGLE_360;
		return radians;
	}

	inline float			wrap_angle180(							float				radians)
	{
		radians = fmod(radians, ANGLE_360);
		if(radians > ANGLE_180)		return radians - ANGLE_360;
		if(radians < -ANGLE_180)	return radians + ANGLE_360;
		return radians;
	}

	float					angle_between_0_and_360(				Vec2				ref, 
																	Vec2				V);
}

namespace std 
{
	template <>
	struct hash<cml::Vec2>
	{
		static size_t calculate(const cml::Vec2& point)
		{
			return (hash<float>()(point.x) ^ hash<float>()(point.y) << 1);
		}

		size_t operator()(const cml::Vec2& point) const
		{
			return calculate(point);
		}
	};

	template <>
	struct hash<cml::Vec3>
	{
		static size_t calculate(const cml::Vec3& point)
		{
			return ((hash<float>()(point.x)
				  ^ (hash<float>()(point.y) << 1)) >> 1)
				  ^ (hash<float>()(point.z) << 1);
		}

		size_t operator()(const cml::Vec3& point) const
		{
			return calculate(point);
		}
	};

	template<> 
	struct less<cml::Vec2>
	{
		static bool check(const cml::Vec2& lhs, const cml::Vec2& rhs)
		{
			if(lhs.x > rhs.x)
				return false;

			if(lhs.x < rhs.x)
				return true;

			return lhs.y < rhs.y;
		}

		bool operator() (const cml::Vec2& lhs, const cml::Vec2& rhs) const
		{
			return check(lhs, rhs);
		}
	};

	template<> 
	struct less<cml::Vec3>
	{
		static bool check(const cml::Vec3& lhs, const cml::Vec3& rhs)
		{
			if(lhs.x > rhs.x)
				return false;

			if(lhs.x < rhs.x)
				return true;

			if(lhs.y > rhs.y)
				return false;

			if(lhs.y < rhs.y)
				return true;

			return lhs.z < rhs.z;
		}

		bool operator() (const cml::Vec3& lhs, const cml::Vec3& rhs) const
		{
			return check(lhs, rhs);
		}
	};
}