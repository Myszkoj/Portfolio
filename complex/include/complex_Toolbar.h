#pragma once


#include "complex_Toolset.h"


namespace complex
{
	/*
		Interface for toolset widgets.
	*/
	class Toolbar	: public Widget
					, public dpl::Chain<Toolbar, Toolset>
	{
	private: // subtypes
		using MyToolsets = dpl::Chain<Toolbar, Toolset>;

	public: // data
		dpl::ReadOnly<ImVec2,	Toolbar> position;
		dpl::ReadOnly<float,	Toolbar> verticalPadding;

	public: // lifecycle
		CLASS_CTOR			Toolbar(		const std::string&		NAME,
											const float				HEIGHT,
											const float				VERTICAL_PADDING = 0.f)
			: Widget(NAME, ImVec2(0.f, HEIGHT))
			, verticalPadding(glm::max(0.f, VERTICAL_PADDING))
		{

		}

		CLASS_CTOR			Toolbar(		const Toolbar&			OTHER) = delete;

		CLASS_CTOR			Toolbar(		Toolbar&&				other) noexcept = default;

		Toolbar&			operator=(		const Toolbar&			OTHER) = delete;

		Toolbar&			operator=(		Toolbar&&				other) noexcept = default;

	public: // functions
		inline void			set_position(	const ImVec2&			NEW_POSITION)
		{
			position = NEW_POSITION;
		}

		inline void			add_toolset(	Toolset&				newToolset)
		{
			MyToolsets::attach_back(newToolset);
		}

		void				update();
	};
}