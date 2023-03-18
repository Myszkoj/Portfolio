#pragma once


#include "cml_utilities.h"


namespace cml
{
	class	HVScale
	{
	public: // data
		float horizontal;
		float vertical;

	public: // lifecycle
		CLASS_CTOR		HVScale(	const float		HORIZONTAL,
									const float		VERTICAL)
			: horizontal(HORIZONTAL)
			, vertical(VERTICAL)
		{

		}

		inline HVScale&	operator+=(	const HVScale&	OTHER)
		{
			horizontal	+= OTHER.horizontal;
			vertical	+= OTHER.vertical;
			return *this;
		}

		inline HVScale&	operator*=(	const float		FACTOR)
		{
			horizontal *= FACTOR;
			vertical *= FACTOR;
			return *this;
		}

		inline HVScale&	operator/=(	const float		FACTOR)
		{
			horizontal /= FACTOR;
			vertical /= FACTOR;
			return *this;
		}

		inline HVScale  operator*(	const float		FACTOR) const
		{
			return HVScale(*this) *= FACTOR;
		}

		inline HVScale  operator/(	const float		FACTOR) const
		{
			return HVScale(*this) /= FACTOR;
		}

	public: // functions
		inline void reset()
		{
			horizontal	= 0.f;
			vertical	= 0.f;
		}

		inline void invalidate()
		{
			horizontal	= std::numeric_limits<float>::infinity();
			vertical	= std::numeric_limits<float>::infinity();
		}
	};


	using	HVSize = HVScale;


	/*
		3D coordinates converted into position on the horizontal(XZ plane) at altitude = VPos.
	*/
	class	HVPoint
	{
	public: // data
		glm::vec2	h; // Position on the horizontal(XZ) plane.
		float		v; // Vertical offset from the horizontal plane.

	public: // lifecycle
		CLASS_CTOR			HVPoint()
			: h(0.f, 0.f)
			, v(0.f)
		{
			
		}

		CLASS_CTOR			HVPoint(			const glm::vec3&		XYZ)
			: h(XYZ.x, -XYZ.z)
			, v(XYZ.y)
		{

		}

		CLASS_CTOR			HVPoint(			const glm::vec2&		H_POS,
												const float				V_POS)
			: h(H_POS)
			, v(V_POS)
		{

		}

		CLASS_CTOR			HVPoint(			const float				HX_POS,
												const float				HY_POS,
												const float				V_POS)
			: h(HX_POS, HY_POS)
			, v(V_POS)
		{

		}

	public: // functions
		inline bool			operator==(			const HVPoint&			OTHER) const
		{
			return h == OTHER.h && v == OTHER.v;
		}
		
		inline bool			operator!=(			const HVPoint&			OTHER) const
		{
			return h != OTHER.h || v != OTHER.v;
		}

		inline HVPoint		operator+(			const HVPoint&			OTHER) const
		{
			return HVPoint(h + OTHER.h, v + OTHER.v);
		}

		inline HVPoint		mod(				const HVScale&			HV) const
		{
			return HVPoint(glm::vec2(fmod(h.x, HV.horizontal), fmod(h.y, HV.horizontal)), fmod(v, HV.vertical));
		}

		inline void			swap(				HVPoint&				other)
		{
			std::swap(h, other.h);
			std::swap(v, other.v);
		}

		inline void			minimize(			const HVPoint&			OTHER)
		{
			if(OTHER.h.x < h.x) h.x = OTHER.h.x;
			if(OTHER.h.y < h.y) h.y = OTHER.h.y;
			if(OTHER.v < v) v = OTHER.v;
		}

		inline void			maximize(			const HVPoint&			OTHER)
		{
			if(OTHER.h.x > h.x) h.x = OTHER.h.x;
			if(OTHER.h.y > h.y) h.y = OTHER.h.y;
			if(OTHER.v > v) v = OTHER.v;
		}

		static HVPoint		min_of(				const HVPoint*			POINTS,
												const uint32_t			NUM_POINTS)
		{
			HVPoint output = POINTS[0];
			for(uint32_t index = 1; index < NUM_POINTS; ++index)
			{
				output.minimize(POINTS[index]);
			}
			return output;
		}

		static HVPoint		max_of(				const HVPoint*			POINTS,
												const uint32_t			NUM_POINTS)
		{
			HVPoint output = POINTS[0];
			for(uint32_t index = 1; index < NUM_POINTS; ++index)
			{
				output.maximize(POINTS[index]);
			}
			return output;
		}

	public: // conversions
		inline operator		const glm::vec3() const
		{
			return this->to_xyz();
		}

		inline glm::vec3	to_xyz() const
		{
			return glm::vec3(h.x, v, -h.y);
		}

		inline glm::vec2	vector_to(			const HVPoint&		OTHER) const
		{
			return OTHER.h - this->h;
		}

		inline glm::vec2	vector_from(		const HVPoint&		OTHER) const
		{
			return this->h - OTHER.h;
		}

		inline float		horizontal_distance(const HVPoint&		OTHER) const
		{
			return glm::distance(this->h, OTHER.h);
		}

		inline float		vertical_distance(	const HVPoint&		OTHER) const
		{
			return glm::abs(this->v - OTHER.v);
		}

		inline void			convert(			const glm::vec3&	NEW_COORDINATES)
		{
			h.x	= NEW_COORDINATES.x;
			v	= NEW_COORDINATES.y;
			h.y	= -NEW_COORDINATES.z;
		}

		inline void			invalidate_horizontal()
		{
			h.x = std::numeric_limits<float>::quiet_NaN();
			h.y = std::numeric_limits<float>::quiet_NaN();
		}

		inline void			invalidate_vertical()
		{
			v = std::numeric_limits<float>::quiet_NaN();
		}

		inline void			invalidate()
		{
			invalidate_horizontal();
			invalidate_vertical();
		}
	};


	class	HVDistance : public HVScale
	{
	public: // lifecycle
		using HVScale::HVScale;

		CLASS_CTOR	HVDistance(): HVScale(0.f, 0.f){}

		CLASS_CTOR	HVDistance(	const HVPoint& A,
								const HVPoint& B)
			: HVScale(glm::distance(A.h, B.h), glm::abs(A.v - B.v))
		{

		}

	public: // functions
		using HVScale::reset;

		inline void reset(		const HVPoint& A,
								const HVPoint& B)
		{
			horizontal	= glm::distance(A.h, B.h);
			vertical	= glm::abs(A.v - B.v);
		}
	};
}