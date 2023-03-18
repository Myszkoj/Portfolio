#include "..//include/complex_Toolbar.h"
#include "../include/complex_Application.h"


namespace complex
{
	void	Toolbar::update()
	{	
		if(bVisible)
		{
			static const ImGuiWindowFlags TOOLBAR_FLAGS	= ImGuiWindowFlags_NoScrollbar 
														| ImGuiWindowFlags_NoSavedSettings 
														| ImGuiWindowFlags_NoTitleBar 
														| ImGuiWindowFlags_NoResize 
														| ImGuiWindowFlags_NoMove 
														| ImGuiWindowFlags_NoCollapse 
														| ImGuiWindowFlags_NoDocking;

			const float SIZE	= requiredSize().y;
			const float LENGTH	= Application::ref().mainWindow()->contentSpace().GetWidth();
			
			resize(LENGTH, SIZE);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(SIZE, SIZE));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, verticalPadding()));

			ImGui::SetNextWindowPos(position());
			ImGui::SetNextWindowSize(requiredSize());

			if(ImGui::Begin(name().c_str(), nullptr, TOOLBAR_FLAGS))
			{
				ImGui::Horizontal();
				ImGui::Separator();

				MyToolsets::for_each_link([&](Toolset& toolset)
				{
					if(toolset.update())
					{
						ImGui::Separator();
					}
				});

				ImGui::Vertical();

			}ImGui::End();

			ImGui::PopStyleVar(2);
		}
	}
}