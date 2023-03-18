#pragma once


#include <dpl_Chain.h>
#include "complex_Application.h"
#include "complex_Widget.h"


namespace complex
{
	class Toolbar;


	/*
		Interface for toolset widgets.
	*/
	class Toolset	: public Widget
					, public dpl::Link<Toolbar, Toolset>
	{
	public: // relations
		friend Toolbar;

	public: // lifecycle
		CLASS_CTOR		Toolset(	const std::string&	NAME)
			: Widget(NAME)
		{

		}

	protected: // functions
		inline float	get_max_height() const
		{
			return ImGui::GetContentRegionAvail().y;
		}

		inline ImVec2	get_control_size() const
		{
			return ImVec2(0.f, get_max_height());
		}

	private: // interface
		/*
			Returns true if toolset is active.
		*/
		virtual bool	update() = 0;
	};
}