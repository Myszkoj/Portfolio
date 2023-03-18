#include "../include/cml_Funnel.h"


namespace cml
{
//=====> Funnel public: // lifecycle
	CLASS_CTOR				Funnel::Funnel()
		: toLeft(0.f, 0.f)
		, toRight(0.f, 0.f)
	{

	}

	CLASS_CTOR				Funnel::Funnel(					const Vec2&				TO_LEFT, 
															const Vec2&				TO_RIGHT)
		: toLeft(TO_LEFT)
		, toRight(TO_RIGHT)
	{

	}

	CLASS_CTOR				Funnel::Funnel(					const Vec2&				APEX,
															const Vec2&				LEFT_END, 
															const Vec2&				RIGHT_END)
		: toLeft(LEFT_END - APEX)
		, toRight(RIGHT_END - APEX)
	{
	}

	CLASS_CTOR				Funnel::Funnel(					const Vec2&				TO_LEFT_CENTER, 
															const Vec2&				TO_RIGHT_CENTER,
															const float				RADIUS)
		: toLeft(TO_LEFT_CENTER)
		, toRight(TO_RIGHT_CENTER)
	{
		set_left_vector(TO_LEFT_CENTER, RADIUS);
		set_right_vector(TO_RIGHT_CENTER, RADIUS);
	}

	CLASS_CTOR				Funnel::Funnel(					const Vec2&				APEX,
															const Vec2&				LEFT_CENTER, 
															const Vec2&				RIGHT_CENTER,
															const float				RADIUS)
		: toLeft(LEFT_CENTER - APEX)
		, toRight(RIGHT_CENTER - APEX)
	{
		set_left_point(LEFT_CENTER, APEX, RADIUS);
		set_right_point(RIGHT_CENTER, APEX, RADIUS);
	}

//=====> Funnel public: // left functions
	void					Funnel::set_left_vector(		const Vec2&				TO_CENTER,
															const float				RADIUS)
	{
		if(const float DISTANCE = calculate_length(TO_CENTER))
		{
			toLeft = calculate_right_tangent(	TO_CENTER / DISTANCE, 
												DISTANCE, 
												RADIUS);
		}
		else
		{
			toLeft->x = 0.f;
			toLeft->y = 0.f;
		}
	}

	Funnel::Operation		Funnel::check_left_vector(		const Vec2&				VECTOR) const
	{
		if(right_side(toLeft, VECTOR))
		{
			if(left_side(toRight, VECTOR))
			{
				return Operation::TIGHTEN;
			}

			return Operation::TWISTED;
		}

		return Operation::WIDTHEN;
	}

	Funnel::Operation		Funnel::tighten_left_vector(	const Vec2&				VECTOR)
	{
		Operation result = check_left_vector(VECTOR);

		if(result == Operation::TIGHTEN)
			toLeft = VECTOR;

		return result;
	}

	Funnel::Operation		Funnel::tighten_left_vector(	const Vec2&				TO_CENTER,
															const float				RADIUS)
	{
		if(const float DISTANCE = calculate_length(TO_CENTER))
		{
			const Vec2 TANGENT = calculate_right_tangent(	TO_CENTER / DISTANCE, 
															DISTANCE, 
															RADIUS);

			return tighten_left_vector(TANGENT);
		}
		
		toLeft->x = 0.f;
		toLeft->y = 0.f;
		return Operation::TIGHTEN;
	}

//=====> Funnel public: // right functions
	void					Funnel::set_right_vector(		const Vec2&				TO_CENTER,
															const float				RADIUS)
	{
		if(const float DISTANCE = calculate_length(TO_CENTER))
		{
			toRight = calculate_left_tangent(	TO_CENTER / DISTANCE, 
												DISTANCE, 
												RADIUS);
		}
		else
		{
			toRight->x = 0.f;
			toRight->y = 0.f;
		}
	}

	Funnel::Operation		Funnel::check_right_vector(		const Vec2&				VECTOR) const
	{
		if(left_side(toRight, VECTOR))
		{
			if(right_side(toLeft, VECTOR))
			{
				return Operation::TIGHTEN;
			}

			return Operation::TWISTED;
		}

		return Operation::WIDTHEN;
	}

	Funnel::Operation		Funnel::tighten_right_vector(	const Vec2&				VECTOR)
	{
		Operation result = check_right_vector(VECTOR);

		if(result == Operation::TIGHTEN)
			toRight = VECTOR;

		return result;
	}

	Funnel::Operation		Funnel::tighten_right_vector(	const Vec2&				TO_CENTER,
															const float				RADIUS)
	{
		if(const float DISTANCE = calculate_length(TO_CENTER))
		{
			const Vec2 TANGENT = calculate_left_tangent(TO_CENTER / DISTANCE, 
														DISTANCE, 
														RADIUS);

			return tighten_right_vector(TANGENT);
		}

		toRight->x = 0.f;
		toRight->y = 0.f;
		return Operation::TIGHTEN;
	}

//=====> Funnel public: // other functions
	Side					Funnel::vector_side(			const Vec2&				FROM_APEX) const
	{
		if(right_side_equal(toRight, FROM_APEX))
			return Side::eRIGHT;

		if(left_side_equal(toLeft, FROM_APEX))
			return Side::eLEFT;

		return Side::eNONE;
	}

	std::optional<Funnel>	Funnel::calculate_common(		const Funnel&			OTHER) const
	{
		if(OTHER.vector_inside_equal(toLeft()))
		{
			if(OTHER.vector_inside_equal(toRight()))
			{
				return *this;
			}
			else // bRIGHT_OUTSIDE
			{
				return Funnel(toLeft(), OTHER.toRight());
			}
		}
		else // bLEFT_OUTSIDE
		{
			if(OTHER.vector_inside_equal(toRight()))
			{
				return Funnel(OTHER.toLeft(), toRight());
			}
		}

		return std::nullopt;
	}
}