#include "..//include/complex_PopupWindow.h"


namespace complex
{
	CLASS_CTOR	PopupWindow::PopupWindow(		const Type				TYPE,
												const std::string&		NAME,
												const ImVec2&			REQUIRED_SIZE)
		: Window(NAME, REQUIRED_SIZE)
		, type(TYPE)
		, state(CLOSED)
	{
		set_AlwaysAutoResize(true);
		set_NoResize(true);
	}

	bool		PopupWindow::reopen(			const ImGuiPopupFlags	FLAGS)
	{
		if (ImGui::IsMouseReleased(FLAGS & ImGuiPopupFlags_MouseButtonMask_))
		{
			if(state == State::OPEN)
			{
				show();
				IM_ASSERT(hash() != 0);
				ImGui::OpenPopupEx(hash(), FLAGS);
			}
			else if(state == State::CLOSED)
			{
				state = State::OPEN_REQUESTED;
			}

			return true;
		}

		return false;
	}

	void		PopupWindow::update()
	{
		if(WindowPadding) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, WindowPadding.value());

		if(state() == State::OPEN_REQUESTED)
		{
			ImGui::OpenPopupEx(hash());
			show(); // We have to make this popup visible because using X to exit the popup sets bVisible to false.
		}

		bool bWasOpen = false;

		if(type() == Type::CONTEXT)
		{
			if(ImGui::BeginPopupEx(hash(), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
			{
				state = State::OPEN;
				Window::update();
				ImGui::EndPopup();
				bWasOpen = true;
			}
			else
			{
				state = State::CLOSED;
			}
		}
		else // MODAL (Slightly modified code from ImGui::BeginPopupModal)
		{
			ImGuiContext& g = *GImGui;
			if (ImGui::IsPopupOpen(hash(), ImGuiPopupFlags_None))
			{
				state = State::OPEN;

				if ((g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasPos) == 0)
				{
					const ImGuiViewport* VIEWPORT = g.CurrentWindow->WasActive	? g.CurrentWindow->Viewport 
																				: ImGui::GetMainViewport();

					ImGui::SetNextWindowPos(VIEWPORT->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
				}

				const ImGuiWindowFlags INNER_FLAGS	= Window::flags() 
													| ImGuiWindowFlags_Popup 
													| ImGuiWindowFlags_Modal 
													| ImGuiWindowFlags_NoCollapse 
													| ImGuiWindowFlags_NoDocking;
				
				bool bXState = bVisible();
				if(ImGui::Begin(name().c_str(), &bXState, INNER_FLAGS))
				{
					if(bXState)
					{
						Window::update();
						ImGui::EndPopup();
					}
					else // User clicked X button.
					{
						ImGui::EndPopup();
						ImGui::ClosePopupToLevel(g.BeginPopupStack.Size, true);
					}
					bWasOpen = true;
				}
				else // We called Begin during first "if" statement, now we have to close the popup.
				{
					hide();
					ImGui::EndPopup();
				}
			}
			else // We behave like Begin() and need to consume those values.
			{
				state = State::CLOSED;
				g.NextWindowData.ClearFlags();
			}
		}

		if(WindowPadding) ImGui::PopStyleVar(1);

		if(bWasOpen)
		{
			if(!ImGui::IsPopupOpen(this->hash(), ImGuiPopupFlags_None))
			{
				on_closed();
			}
		}
	}
}