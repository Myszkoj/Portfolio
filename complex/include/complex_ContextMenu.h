#pragma once


#include "complex_PopupWindow.h"


namespace complex
{
	/*
		Popup context menu.
		Adds helper function that renders selectables with horizontal offset that leavs space for icons.
	*/
	class ContextMenu : public PopupWindow
	{
	public: // constants
		static const float MIN_ICON_SPACE;

	public: // data
		dpl::ReadOnly<float, ContextMenu> iconSpace;

	public: // lifecycle
		CLASS_CTOR      ContextMenu(	const std::string&		NAME,
										const float				ICON_SPACE = MIN_ICON_SPACE);

	public: // functions
		using Window::set_Content;

		inline void		select_option(	const char*				OPTION_NAME,
										const char*				OPTION_ICON,
										const ImU32*			ICON_COLOR,
										std::function<void()>	on_selected)
		{
			handle_icon(OPTION_ICON, ICON_COLOR);     
			if(ImGui::Selectable(OPTION_NAME))
			{
				on_selected();
			}
		}

		inline void		select_option(	const char*				OPTION_NAME,
										std::function<void()>	on_selected)
		{
			select_option(OPTION_NAME, nullptr, nullptr, on_selected);
		}

		inline void		select_menu(	const char*				MENU_NAME,
										const char*				MENU_ICON,
										const ImU32*			ICON_COLOR,
										std::function<void()>	on_menu_open)
		{
			handle_icon(MENU_ICON, ICON_COLOR);  
			if(ImGui::BeginMenu(MENU_NAME))
			{
				on_menu_open();
				ImGui::EndMenu();
			}
		}

		inline void		select_menu(	const char*				MENU_NAME,
										std::function<void()>	on_menu_open)
		{
			select_menu(MENU_NAME, nullptr, nullptr, on_menu_open);
		}

	private: // functions
		void			handle_icon(	const char*				ICON,
										const ImU32*			ICON_COLOR);
	};
}