#include "..//include/complex_Dockable.h"


namespace complex
{
	void		Dockable::dock_child(			Dockable&				child,
												const ImGuiDir_			DIRECTION,
												const float				RATIO)
	{
		if(this != &child)
		{
			if(MyChildren::attach_back(child))
			{
				if(DIRECTION == ImGuiDir_None || DIRECTION == ImGuiDir_COUNT)
					throw dpl::GeneralException(this, __LINE__, "Invalid direction.");

				if(RATIO < 0.f || RATIO > 1.f)
					throw dpl::GeneralException(this, __LINE__, "Invalid ratio.");

				*child.direction	= DIRECTION;
				*child.ratio		= RATIO;
			}
		}
	}

	void		Dockable::update_docking(		ImGuiID&				dockID)
	{
		MyChildren::iterate_forward([&](Dockable& child)
		{
			if(child.direction() != ImGuiDir_None)
			{
				// https://github.com/ocornut/imgui/issues/3328
				//ImGui::DockBuilderSetNodeSize(dockID, ImVec2(100.f, 100.f));

				ImGuiID			childID	= ImGui::DockBuilderSplitNode(dockID, child.direction(), child.ratio(), nullptr, &dockID);
				ImGuiDockNode*	node	= ImGui::DockBuilderGetNode(childID);
								node->LocalFlags = child.flags();

				child.update_docking(childID);
			}
		});

		ImGui::DockBuilderDockWindow(get_target_name(), dockID);
	}
}