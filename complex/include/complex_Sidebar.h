#pragma once


#include "complex_Widget.h"


namespace complex
{
	class Sidebar
	{
	public: // subtypes
		using	Content	= std::function<void()>;

	public: // data
		dpl::ReadOnly<std::string,	Sidebar>	name;
		dpl::ReadOnly<ImGuiDir_,	Sidebar>	location;
		dpl::ReadOnly<Content,		Sidebar>	content;

	public: // lifecycle
		CLASS_CTOR			Sidebar(		const std::string&		NAME,
											const ImGuiDir_			LOCATION)
			: name(NAME)
			, location(LOCATION)
		{

		}

		CLASS_CTOR			Sidebar(		const Sidebar&			OTHER) = delete;
		CLASS_CTOR			Sidebar(		Sidebar&&				other) noexcept = default;
		Sidebar&			operator=(		const Sidebar&			OTHER) = delete;
		Sidebar&			operator=(		Sidebar&&				other) noexcept = default;

	public: // functions
		void				set_content(	const Content			CONTENT)
		{
			content = CONTENT;
		}

		void				update(			ImGuiViewport*			viewport)
		{
			static const ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
			if(ImGui::BeginViewportSideBar(name().c_str(), viewport, location, ImGui::GetFrameHeight(), WINDOW_FLAGS))
			{
				if(ImGui::BeginMenuBar()) 
				{
					if(content()) content()();
					ImGui::EndMenuBar();
				}
				ImGui::End();
			}
		}
	};
}