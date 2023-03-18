#pragma once


#include <optional>
#include <dpl_ReadOnly.h>
#include "cml_utilities.h"


namespace cml
{
	class Funnel
	{
	public: // subtypes
		enum class Operation : char
		{
			WIDTHEN,	// angle between left and right is greater
			TWISTED,	// directions are swapped
			TIGHTEN,	// new vector is between old vectors
			INVALID
		};

	public: // data
		dpl::ReadOnly<Vec2, Funnel> toLeft;
		dpl::ReadOnly<Vec2, Funnel> toRight;

	public: // CTOR/DTOR
		CLASS_CTOR					Funnel();

		CLASS_CTOR					Funnel(					const Vec2&				TO_LEFT, 
															const Vec2&				TO_RIGHT);

		CLASS_CTOR					Funnel(					const Vec2&				APEX,
															const Vec2&				LEFT_END, 
															const Vec2&				RIGHT_END);

		CLASS_CTOR					Funnel(					const Vec2&				TO_LEFT_CENTER, 
															const Vec2&				TO_RIGHT_CENTER,
															const float				RADIUS);

		CLASS_CTOR					Funnel(					const Vec2&				APEX,
															const Vec2&				LEFT_CENTER, 
															const Vec2&				RIGHT_CENTER,
															const float				RADIUS);

	public: // left functions
		inline void					set_left_vector(		const Vec2&				NEW_VECTOR)
		{
			*toLeft = NEW_VECTOR;
		}

		void						set_left_vector(		const Vec2&				TO_CENTER,
															const float				RADIUS);

		inline void					set_left_point(			const Vec2&				POINT,
															const Vec2&				APEX)
		{
			*toLeft = POINT - APEX;
		}

		inline void					set_left_point(			const Vec2&				CENTER,
															const Vec2&				APEX,
															const float				RADIUS)
		{
			set_left_vector(CENTER - APEX, RADIUS);
		}

		Operation					check_left_vector(		const Vec2&				VECTOR) const;

		Operation					tighten_left_vector(	const Vec2&				VECTOR);

		Operation					tighten_left_vector(	const Vec2&				TO_CENTER,
															const float				RADIUS);

		inline Operation			tighten_left_point(		const Vec2&				POINT,
															const Vec2&				APEX)
		{
			return tighten_left_vector(POINT - APEX);
		}

		inline Operation			tighten_left_point(		const Vec2&				CENTER,
															const Vec2&				APEX,
															const float				RADIUS)
		{
			return tighten_left_vector(CENTER - APEX, RADIUS);
		}

	public: // right functions
		inline void					set_right_vector(		const Vec2&				NEW_VECTOR)
		{
			*toRight = NEW_VECTOR;
		}

		void						set_right_vector(		const Vec2&				TO_CENTER,
															const float				RADIUS);

		inline void					set_right_point(		const Vec2&				POINT,
															const Vec2&				APEX)
		{
			*toRight = POINT - APEX;
		}

		inline void					set_right_point(		const Vec2&				CENTER,
															const Vec2&				APEX,
															const float				RADIUS)
		{
			set_right_vector(CENTER - APEX, RADIUS);
		}

		Operation					check_right_vector(		const Vec2&				VECTOR) const;

		Operation					tighten_right_vector(	const Vec2&				VECTOR);

		Operation					tighten_right_vector(	const Vec2&				TO_CENTER,
															const float				RADIUS);

		inline Operation			tighten_right_point(	const Vec2&				POINT,
															const Vec2&				APEX)
		{
			return tighten_right_vector(POINT - APEX);
		}

		inline Operation			tighten_right_point(	const Vec2&				CENTER,
															const Vec2&				APEX,
															const float				RADIUS)
		{
			return tighten_right_vector(CENTER - APEX, RADIUS);
		}

	public: // other functions
		inline bool					set_vector(				const Vec2&				NEW_VECTOR,
															const Side				SIDE)
		{
			switch(SIDE)
			{
			case Side::eRIGHT:	set_right_vector(NEW_VECTOR);	return true;
			case Side::eLEFT:	set_left_vector(NEW_VECTOR);	return true;
			default: return false;
			}
		}

		inline bool					set_point(				const Vec2&				POINT,
															const Vec2&				APEX,
															const Side				SIDE)
		{
			return set_vector(POINT - APEX, SIDE);
		}

		inline Operation			check_vector(			const Vec2&				NEW_VECTOR,
															const Side				SIDE) const
		{
			switch(SIDE)
			{
			case Side::eRIGHT:	return check_right_vector(NEW_VECTOR);
			case Side::eLEFT:	return check_left_vector(NEW_VECTOR);
			default: return Operation::INVALID;
			}
		}

		inline Operation			tighten_vector(			const Vec2&				NEW_VECTOR,
															const Side				SIDE)
		{
			switch(SIDE)
			{
			case Side::eRIGHT:	return tighten_right_vector(NEW_VECTOR);
			case Side::eLEFT:	return tighten_left_vector(NEW_VECTOR);
			default: return Operation::INVALID;
			}
		}

		inline Operation			tighten_vector(			const Vec2&				TO_CENTER,
															const float				RADIUS,
															const Side				SIDE)
		{
			switch(SIDE)
			{
			case Side::eRIGHT:	return tighten_right_vector(TO_CENTER, RADIUS);
			case Side::eLEFT:	return tighten_left_vector(TO_CENTER, RADIUS);
			default: return Operation::INVALID;
			}
		}

		Side						vector_side(			const Vec2&				FROM_APEX) const;

		inline bool					vector_inside(			const Vec2&				VECTOR) const
		{
			return vector_side(VECTOR) == Side::eNONE;
		}

		inline bool					vector_inside_equal(	const Vec2&				VECTOR) const
		{
			return left_side_equal(toRight(), VECTOR) 
				&& right_side_equal(toLeft(), VECTOR);
		}

		inline Side					point_side(				const Vec2&				POINT,
															const Vec2&				APEX) const
		{
			return vector_side(POINT - APEX);
		}

		inline bool					point_inside(			const Vec2&				POINT,
															const Vec2&				APEX) const
		{
			return vector_side(POINT - APEX) == Side::eNONE;
		}

		inline bool					point_inside_equal(		const Vec2&				POINT,
															const Vec2&				APEX) const
		{
			return vector_inside_equal(POINT - APEX);
		}

		inline bool					is_degenerate() const
		{
			return left_side(toLeft(), toRight());
		}

		inline bool					is_collapsed() const
		{
			if(calculate_dot(toLeft(), toRight()) < 0.f) return false;
			return glm::abs(calculate_det(toLeft(), toRight())) < glm::epsilon<float>();
		}

		std::optional<Funnel>		calculate_common(		const Funnel&			OTHER) const;

		inline Funnel				calculate_inverse() const
		{
			return Funnel(-toLeft(), -toRight());
		}
	};
}