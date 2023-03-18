#include "..//include/complex_Dockspace.h"


namespace complex
{
	CLASS_CTOR	Dockspace::Dockspace(	const std::string&		NAME,
										const ImVec2&			REQUIRED_SIZE)
		: Window(NAME, REQUIRED_SIZE)
		, dockspaceID(ImHashStr((name() + "##target").c_str()))
		, bNeedsRedock(false)
	{
		Window::set_NoDocking(true);
		set_NoWindowMenuButton(true);
		set_NoTitleBar(true);
		set_NoCollapse(true);
		set_NoMove(true);
		set_NoResize(true);
		set_NoBringToFrontOnFocus(true);
		set_NoNavFocus(true);
	}

	void		Dockspace::update()
	{
		ImGui::SetNextWindowPos(offset());
		ImGui::SetNextWindowSize(requiredSize());

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,	0.0f);				// 1)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);				// 2)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,	ImVec2(0.0f, 0.0f));// 3)

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::Begin(name().c_str(), nullptr, Window::flags());
		ImGuiIO& io = ImGui::GetIO();
		if((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) == 0x00000000)
			throw dpl::GeneralException(this, __LINE__, "Docking disabled.");

		if(bNeedsRedock || !ImGui::DockBuilderGetNode(dockspaceID()))
		{
			// Designing already docked space: https://github.com/ocornut/imgui/issues/2583		
			ImGui::DockBuilderAddNode(dockspaceID(), Dockable::flags());
			ImGuiID mainID = dockspaceID;	
			Dockable::update_docking(mainID);	// Note that dockspaceID is passed as reference, imgui may change that id.
			ImGui::DockBuilderFinish(mainID);

			bNeedsRedock = false;
		}
		
		ImGui::DockSpace(dockspaceID, ImVec2(0.f, 0.f), Dockable::flags());
		ImGui::End();

		ImGui::PopStyleVar(3);
	}
}