#pragma once


#include <string>
#include <dpl_Range.h>
#include "complex_Widget.h"


namespace complex
{
	template<uint32_t N, typename T>
	class DragVector	: public Widget
						, public ValueControl<glm::vec<N, T>>
	{
	private: // subtypes
		using MyControlBase = ValueControl<glm::vec<N, T>>;

	public: // data
		dpl::ReadOnly<dpl::Range<T>,	DragVector> range;
		dpl::ReadOnly<float,			DragVector> speed;
		dpl::ReadOnly<const char*,		DragVector> format;

	public: // lifecycle
		CLASS_CTOR		DragVector(	const std::string&		NAME,
									const dpl::Range<T>&	RANGE,
									const float				SPEED,
									const char*				FORMAT)
			: Widget(NAME)
			, MyControlBase(glm::vec<N, T>(RANGE.min()))
			, range(RANGE)
			, speed(glm::max(0.f, SPEED))
			, format(FORMAT)
		{
			
		}

	private: // functions
		virtual bool	on_update(	glm::vec<N, T>&			currentValue) final override
		{
			return ImGui::DragScalarN(name().c_str(), TypeToImGuiTypeID<T>::ID, &currentValue[0], N, speed(), &range().min(), &range().max(), format(), ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
		}
	};


	template<typename T>
	class DragVector<1, T>	: public Widget
							, public ValueControl<T>
	{
	private: // subtypes
		using MyControlBase = ValueControl<T>;

	public: // data
		dpl::ReadOnly<dpl::Range<T>,	DragVector> range;
		dpl::ReadOnly<float,			DragVector> speed;
		dpl::ReadOnly<const char*,		DragVector> format;

	public: // lifecycle
		CLASS_CTOR		DragVector(	const std::string&		NAME,
									const dpl::Range<T>&	RANGE,
									const float				SPEED,
									const char*				FORMAT)
			: Widget(NAME)
			, MyControlBase(RANGE.min())
			, range(RANGE)
			, speed(glm::max(0.f, SPEED))
			, format(FORMAT)
		{
			
		}

	private: // functions
		virtual bool	on_update(	T&					currentValue) final override
		{
			return ImGui::DragScalar(name().c_str(), TypeToImGuiTypeID<T>::ID, &currentValue, speed(), &range().min(), &range().max(), format(), ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
		}
	};


	using DragInt		= DragVector<1, int32_t>;
	using DragUInt		= DragVector<1, uint32_t>;
	using DragFloat		= DragVector<1, float>;
	using DragDouble	= DragVector<1, double>;
}