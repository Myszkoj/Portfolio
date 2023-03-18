#include "..//include/complex_ContentWindow.h"


// lifecycle
namespace complex
{
	CLASS_CTOR	ContentWindow::ContentWindow(		const std::string&		NAME,
													const std::string		HEADER_FRAME_STR,
													const std::string		HEADER_CONTENT_STR,
													const ImVec2&			REQUIRED_SIZE,
													const bool				bHAS_BORDER)
		: Window(NAME, REQUIRED_SIZE)
		, rounding(4.f)
		, cornerFlags(ImDrawCornerFlags_All)
		, bHasBorder(bHAS_BORDER)
		, bCollapsed(false)
		, headerFrameHash(ImHashStr(HEADER_FRAME_STR.c_str(), HEADER_FRAME_STR.size()))
		, headerContentHash(ImHashStr(HEADER_CONTENT_STR.c_str(), HEADER_CONTENT_STR.size()))
	{

	}

	CLASS_CTOR	ContentWindow::ContentWindow(	    const std::string&		NAME,
													const ImVec2&			REQUIRED_SIZE,
													const bool			    bHAS_BORDER)
		: ContentWindow(NAME, NAME + "_FRAME", NAME + "_CONTENT", REQUIRED_SIZE, bHasBorder)
	{

	}
}

// private functions
namespace complex
{
	bool        ContentWindow::begin_collapsing()
	{
		const float     WINDOW_WIDTH    = ImGui::GetWindowContentRegionWidth();
		const float     FRAME_HEIGHT    = ImGui::GetFrameHeight();

		const ImVec2    RECT_MIN        = ImGui::GetCursorScreenPos();
		const ImVec2    RECT_MAX        = RECT_MIN + ImVec2(WINDOW_WIDTH, FRAME_HEIGHT);

		ImGui::BeginGroup();

		{ImGui::BeginGroup();

			ImGui::PushID(headerFrameHash());

			const auto              FRAME_COLOR     = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
			const ImDrawCornerFlags CORNER_ROUNDING = cornerFlags() & ImDrawCornerFlags_Top;

			ImDrawList* drawList = ImGui::GetWindowDrawList();
						drawList->AddRectFilled(RECT_MIN, RECT_MAX, FRAME_COLOR, rounding(), CORNER_ROUNDING);

			ImGui::PushClipRect(RECT_MIN, RECT_MAX, true);
			render_frame_content();
			ImGui::PopClipRect();

			ImGui::PopID();

		}ImGui::EndGroup();

		const float OFFSET = bCollapsed() ? 0.f : GImGui->Style.ItemSpacing.y;
		ImGui::ItemSize(ImVec2(WINDOW_WIDTH, FRAME_HEIGHT - OFFSET));

		if (!bCollapsed())
		{
			const ImGuiWindow*      PARENT_WINDOW   = GImGui->CurrentWindow;
			const ImGuiWindowFlags  WINDOW_FLAGS    = ImGuiWindowFlags_NoDecoration 
													| ImGuiWindowFlags_NoTitleBar 
													| ImGuiWindowFlags_NoResize 
													| ImGuiWindowFlags_NoSavedSettings 
													| ImGuiWindowFlags_ChildWindow 
													| ImGuiWindowFlags_NoDocking 
													| ImGuiWindowFlags_NoScrollbar 
													| ImGuiWindowFlags_NoScrollWithMouse 
													| ImGuiWindowFlags_AlwaysUseWindowPadding
													| ImGuiWindowFlags_AlwaysAutoResize // TEST!!!!!!!!!!!!
													| (PARENT_WINDOW->Flags & ImGuiWindowFlags_NoMove);  // Inherit the NoMove flag

			ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, -1.f));

			bCollapsed = !ImGui::Begin(name().c_str(), NULL, WINDOW_FLAGS);
		}

		return !bCollapsed;
	}

	void		ContentWindow::render_frame_content()
	{
		ImGui::BeginGroup();
		ImGui::PushID(headerContentHash());

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		float getFrameHeight = ImGui::GetFrameHeight();
		float removedSize = 5.5f; // reduce few pixels
		float halfSize = (getFrameHeight) / 2.0f - removedSize;

		const ImVec2 SCREEN_POS = ImGui::GetCursorScreenPos();

		float arrowPaddingLeft = 6;

		//----------------------X-------------------------------------------Y-----------------------------------
		const ImVec2 ORIGIN(    SCREEN_POS.x + halfSize + arrowPaddingLeft, SCREEN_POS.y + getFrameHeight / 2.0f);
		const ImVec2 RECT_MIN(  ORIGIN.x - halfSize - removedSize,          ORIGIN.y - halfSize - removedSize);
		const ImVec2 RECT_MAX(  ORIGIN.x + halfSize + removedSize,          ORIGIN.y + halfSize + removedSize);

		const auto TEXT_COLOR       = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_Text]);
		const auto HOVERED_COLOR    = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
		const auto CLICK_COLOR      = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);

		if(ImGui::IsWindowHovered()) 
		{
			if(ImGui::IsMouseHoveringRect(RECT_MIN, RECT_MAX)) 
			{
				if (ImGui::IsMouseDown(0))
				{
					drawList->AddCircleFilled(ORIGIN, halfSize * 2, CLICK_COLOR);
				}
				else
				{
					if (ImGui::IsMouseReleased(0))
					{
						bCollapsed = !bCollapsed();
					}

					drawList->AddCircleFilled(ORIGIN, halfSize * 2, HOVERED_COLOR);
				}
			}
		}

		float triA_X = 0;
		float triA_Y = 0;
		float triB_X = 0;
		float triB_Y = 0;
		float triC_X = 0;
		float triC_Y = 0;

		if(bCollapsed()) 
		{
			// arrow right
			triA_X = ORIGIN.x - halfSize;
			triA_Y = ORIGIN.y - halfSize;
			triB_X = ORIGIN.x + halfSize;
			triB_Y = ORIGIN.y;
			triC_X = ORIGIN.x - halfSize;
			triC_Y = ORIGIN.y + halfSize;
		}
		else 
		{
			// arrow down
			float offset = -0.5f;
			triA_X = ORIGIN.x - halfSize + offset;
			triA_Y = ORIGIN.y - halfSize;
			triB_X = ORIGIN.x + halfSize + offset;
			triB_Y = ORIGIN.y - halfSize;
			triC_X = ORIGIN.x + offset;
			triC_Y = ORIGIN.y + halfSize;
		}

		const ImVec2 TEXT_SIZE = ImGui::CalcTextSize(name().c_str(), NULL);
		const ImVec2 TEXT_POSITION( ORIGIN.x + 5 + halfSize * 2,                            // X
									SCREEN_POS.y + (getFrameHeight - TEXT_SIZE.y) / 2.0f);  // Y

		//WINDOW_WIDTH    = PARENT_WINDOW->ContentRegionRect.Max.x - PARENT_WINDOW->Pos.x - GImGui->Style.WindowPadding.x;

		drawList->AddText(TEXT_POSITION, TEXT_COLOR, name().c_str(), NULL);
		drawList->AddTriangleFilled(ImVec2(triA_X, triA_Y), ImVec2(triB_X, triB_Y), ImVec2(triC_X, triC_Y), TEXT_COLOR);
	
		ImGui::PopID();
		ImGui::EndGroup();
	}

	void        ContentWindow::end_collapsing()
	{
		ImGuiWindow* childWindow    = NULL;
		ImGuiWindow* parentWindow   = NULL;

		if(!bCollapsed()) 
		{
			childWindow = GImGui->CurrentWindow;

			ImGui::EndChild();
		}

		parentWindow = GImGui->CurrentWindow;

		float bk = GImGui->Style.ItemSpacing.y;

		if (bCollapsed())
			GImGui->Style.ItemSpacing.y = 0;
		ImGui::EndGroup();
		if (bCollapsed())
			GImGui->Style.ItemSpacing.y = bk;

		const ImVec2 SCREEN_POS(parentWindow->DC.CursorPos.x,
								parentWindow->DC.CursorPosPrevLine.y);

		const float HEIGHT			= (childWindow != NULL)  ? (childWindow->Size.y + SCREEN_POS.y + ImGui::GetFrameHeight()) : (SCREEN_POS.y + ImGui::GetFrameHeight());

		const float WINDOW_WIDTH    = ImGui::GetWindowContentRegionWidth();
		const ImU32 BORDER_COLOR    = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_Border]);

		ImDrawList* drawList = ImGui::GetWindowDrawList();
					drawList->AddRect(SCREEN_POS, ImVec2(SCREEN_POS.x + WINDOW_WIDTH, HEIGHT), BORDER_COLOR, rounding(), cornerFlags, 1.0f);

		//ImGui::PopClipRect();
		//ImGui::PopStyleVar();
	}
}

// window implementation
namespace complex
{
	void		ContentWindow::update()
	{
		if(bVisible)
		{
			if(WindowPadding) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, WindowPadding.value());

			if(flags() & ImGuiWindowFlags_NoCollapse)
			{
				if(ImGui::BeginChild(name().c_str(), get_size(), bHasBorder(), Window::flags()))
				{
					Window::update();
				}

				ImGui::EndChild();
			}
			else
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.f);

				if(begin_collapsing())
				{
					Window::update();
				}

				end_collapsing();

				ImGui::PopStyleVar();
			}

			if(WindowPadding) ImGui::PopStyleVar(1);
		}
	}
}