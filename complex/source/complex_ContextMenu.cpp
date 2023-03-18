#include "..//include/complex_ContextMenu.h"


namespace complex
{
//=====> ContextMenu public: // constants
	const float ContextMenu::MIN_ICON_SPACE = 32.f;

//=====> ContextMenu public: // lifecycle
	CLASS_CTOR  ContextMenu::ContextMenu(       const std::string&	NAME,
												const float			ICON_SPACE)
		: PopupWindow(PopupWindow::CONTEXT, NAME)
		, iconSpace(ICON_SPACE)
	{
		WindowPadding   = ImVec2(8, 8);
		FramePadding    = ImVec2(4, 4);
		ItemSpacing     = ImVec2(8, 4);
	}

//=====> ContextMenu private: // functions
	void		ContextMenu::handle_icon(	    const char*			ICON,
												const ImU32*		ICON_COLOR)
	{
		if(ICON)
		{
			if(ICON_COLOR) 
			{
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, *ICON_COLOR);
				ImGui::Text(ICON);
				ImGui::PopStyleColor();
			}
			else // Use text color for the icon.
			{
				ImGui::Text(ICON);
			}
			
			ImGui::SameLine(iconSpace());
		}
		else
		{
			ImGui::SetCursorPos(ImVec2(iconSpace(), ImGui::GetCursorPosY()));
		}
	}
}