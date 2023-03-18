#include "..//include/complex_Window.h"


namespace complex
{
	void	Window::update()
	{
		if(menuBar())
		{
			set_flag(ImGuiWindowFlags_MenuBar, true);

			if(ImGui::BeginMenuBar())
			{
				menuBar()(*this);
				ImGui::EndMenuBar();
			}
		}
		else
		{
			set_flag(ImGuiWindowFlags_MenuBar, false);
		}

		if(content())
		{
			uint32_t numVar = 0;

			if(FramePadding)
			{
				++numVar; ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, FramePadding.value());
			}
			
			if(ItemSpacing)
			{
				++numVar; ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ItemSpacing.value());
			}
					
			if(ItemInnerSpacing)
			{
				++numVar; ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ItemInnerSpacing.value());
			}
			
			content()(*this);

			ImGui::PopStyleVar(numVar);
		}
	}
}