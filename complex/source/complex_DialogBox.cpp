#include "..//include/complex_DialogBox.h"


namespace complex
{
	CLASS_CTOR	DialogBox::DialogBox(		const std::string&		NAME,
											const std::string&		MESSAGE,
											const float				MAX_BUTTON_WIDTH,
											const Alignment			ALIGNMENT)
		: PopupWindow(PopupWindow::MODAL, NAME)
		, message(MESSAGE)
		, alignment(ALIGNMENT)
		, maxButtonWidth(glm::max(16.f, MAX_BUTTON_WIDTH))
	{
		set_NoTitleBar(true);
		set_Content([](Window& thisWindow)
		{
			auto& dialog = static_cast<DialogBox&>(thisWindow);
			if(dialog.message().size() > 0 && dialog.options().size() > 0)
			{
				ImGui::Text(dialog.message().c_str());

				const float		TOTAL_SPACING	= ImGui::GetStyle().ItemSpacing.x * float(dialog.options().size() - 1);
				const float		CONTENT_WIDTH	= ImGui::GetContentRegionAvailWidth();
				const float		BUTTON_WIDTH	= glm::min(dialog.maxButtonWidth(), (CONTENT_WIDTH - TOTAL_SPACING) / dialog.options().size());

				const ImVec2	BUTTON_SIZE(BUTTON_WIDTH, 0);

				if(dialog.alignment() == RIGHT_ALIGNMENT)
				{
					const float TOTAL_SPACE_TAKEN = float(dialog.options().size()) * BUTTON_WIDTH + TOTAL_SPACING;
					const float OFFSET = glm::max(0.f, CONTENT_WIDTH - TOTAL_SPACE_TAKEN);
				
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + OFFSET);
				}

				for(size_t optionID = 0; optionID < dialog.options().size(); ++optionID)
				{
					if(optionID > 0)
					{
						ImGui::SameLine();
					}

					auto& option = dialog.options()[optionID];
					if(ImGui::Button(option.buttonText().c_str(), BUTTON_SIZE))
					{
						if(auto& on_click = option.callback())
						{
							on_click();
						}
					
						ImGui::CloseCurrentPopup();
					}
				}
			}
			else
			{
				ImGui::CloseCurrentPopup();
			}
		});
	}

	CLASS_CTOR	DialogBox::DialogBox(		const std::string&		NAME,
											const float				MAX_BUTTON_WIDTH,
											const Alignment			ALIGNMENT)
		: DialogBox(NAME, "", MAX_BUTTON_WIDTH, ALIGNMENT)
	{

	}
}