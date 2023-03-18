#include "..//include/complex_ChildWindow.h"
//#include "..//include/complex_MainWindow.h"


namespace complex
{
	CLASS_CTOR	ChildWindow::ChildWindow(	const std::string&	NAME,
											const ImVec2&		REQUIRED_SIZE)
		: Window(NAME, REQUIRED_SIZE)
	{

	}

	void		ChildWindow::update()
	{	
		if(bVisible)
		{
			if(WindowPadding) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, WindowPadding.value());

			ImGuiWindowClass	window_class1;
								window_class1.DockNodeFlagsOverrideSet	= Dockable::flags();
								//window_class1.DockingAlwaysTabBar = true;

			ImGui::SetNextWindowClass(&window_class1);

			bool bStayOpen = bVisible();
			const bool bSHOW_CONTENT = ImGui::Begin(name().c_str(), get_private_node_flag(ImGuiDockNodeFlags_NoCloseButton) ? nullptr : &bStayOpen, Window::flags());
			bStayOpen ? show() : hide();

			if(bSHOW_CONTENT)
			{
				Window::update();
			}

			ImGui::End();

			if(WindowPadding) ImGui::PopStyleVar(1);
		}
	}
}