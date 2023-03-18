#include "..//include/complex_ImAddons.h"

// Source:
// https://github.com/ocornut/imgui/issues/1496#issuecomment-655048353
// https://github.com/ocornut/imgui/issues/1901

namespace ImGui
{
	glm::vec3	transform_point(			const glm::mat4&			MVP,
											const glm::vec3&			POINT)
	{
		glm::vec4 tmp = MVP * glm::vec4(POINT, 1.f);
		if (fabsf(tmp.w) > FLT_EPSILON) // check for axis aligned with camera direction
		{
			tmp /= tmp.w;
		}

		return tmp;
	}

	ImVec2		world_to_screen(			const glm::vec3&			WORLD_POSITION, 
											const glm::mat4&			VIEW_PROJECTION, 
											const ImVec2				POSITION, 
											const ImVec2				SIZE)
	{
		// transform from 3D space to NDC
		glm::vec4	tmp = VIEW_PROJECTION * glm::vec4(WORLD_POSITION, 1.f);
					tmp /= tmp.w;

				// transform from <-1, 1> to <0, 1> range
				tmp.x = tmp.x * 0.5f + 0.5f;
				tmp.y = tmp.y * 0.5f + 0.5f;

				// flip Y axis
				tmp.y = 1.f - tmp.y;
				
				// scale 
				tmp.x *= SIZE.x;
				tmp.y *= SIZE.y;

				// translate
				tmp.x += POSITION.x;
				tmp.y += POSITION.y;

		return ImVec2(tmp.x, tmp.y);
	}

	float	get_screen_space_distance(		const glm::vec3&			START, 
											const glm::vec3&			END,
											const glm::mat4&			VIEW_PROJECTION,
											const float					ASPECT_RATIO)
	{
		const glm::vec3 SEGMENT_START    = transform_point(VIEW_PROJECTION, START);
		const glm::vec3 SEGMENT_END      = transform_point(VIEW_PROJECTION, END);

		const glm::vec2 CLIP_SPACE_AXIS((SEGMENT_END.x - SEGMENT_START.x),
										(SEGMENT_END.y - SEGMENT_START.y) / ASPECT_RATIO);

		return cml::calculate_length(CLIP_SPACE_AXIS);
	}

	bool	InputScalarExt(					const char*					label, 
											ImGuiDataType				data_type,
											void*						p_data,
											const void*					p_step,
											const void*					p_step_fast, 
											const char*					format,
											ImGuiInputTextFlags			flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		ImGuiStyle& style = g.Style;

		if (format == NULL)
			format = DataTypeGetInfo(data_type)->PrintFmt;

		static const uint32_t BUFFER_SIZE = 64;

		char buf[BUFFER_SIZE];

		if(flags & ImGuiInputTextFlags_BlankScalar)
		{
			memset(buf, 0, BUFFER_SIZE);
		}
		else
		{
			DataTypeFormatString(buf, IM_ARRAYSIZE(buf), data_type, p_data, format);
		}

		bool value_changed = false;
		if ((flags & (ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsScientific)) == 0)
			flags |= ImGuiInputTextFlags_CharsDecimal;
		flags |= ImGuiInputTextFlags_AutoSelectAll;
		flags |= ImGuiInputTextFlags_NoMarkEdited;  // We call MarkItemEdited() ourselves by comparing the actual data rather than the string.

		if (p_step != NULL)
		{
			const float button_size = GetFrameHeight();

			BeginGroup(); // The only purpose of the group here is to allow the caller to query item data e.g. IsItemActive()
			PushID(label);
			SetNextItemWidth(ImMax(1.0f, CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2));
			if (InputText("", buf, IM_ARRAYSIZE(buf), flags)) // PushId(label) + "" gives us the expected ID from outside point of view
				value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, p_data, format);

			// Step buttons
			const ImVec2 backup_frame_padding = style.FramePadding;
			style.FramePadding.x = style.FramePadding.y;
			ImGuiButtonFlags button_flags = ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups;
			if (flags & ImGuiInputTextFlags_ReadOnly)
				button_flags |= ImGuiButtonFlags_Disabled;
			SameLine(0, style.ItemInnerSpacing.x);
			if (ButtonEx("-", ImVec2(button_size, button_size), button_flags))
			{
				DataTypeApplyOp(data_type, '-', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
				value_changed = true;
			}
			SameLine(0, style.ItemInnerSpacing.x);
			if (ButtonEx("+", ImVec2(button_size, button_size), button_flags))
			{
				DataTypeApplyOp(data_type, '+', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
				value_changed = true;
			}

			const char* label_end = FindRenderedTextEnd(label);
			if (label != label_end)
			{
				SameLine(0, style.ItemInnerSpacing.x);
				TextEx(label, label_end);
			}
			style.FramePadding = backup_frame_padding;

			PopID();
			EndGroup();
		}
		else
		{
			if (InputText(label, buf, IM_ARRAYSIZE(buf), flags))
				value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, p_data, format);
		}
		if (value_changed)
			MarkItemEdited(window->DC.LastItemId);

		return value_changed;
	}

	bool	PrefixedInputVector(			const char*					LABEL,
											const ImGuiDataType			DATA_TYPE,
											void*						p_data, 
											const uint32_t				COMPONENTS, 
											const char**				FORMATS,
											const float*				WIDTH, 
											ImGuiInputTextFlags			flags)
	{
		flags |= ImGuiInputTextFlags_CharsScientific;

		ImGuiContext& g = *GImGui;
		bool value_changed = false;
		ImGui::BeginGroup();
		ImGui::PushID(LABEL);

		WIDTH   ? ImGui::PushMultiItemsWidths(COMPONENTS, *WIDTH)
				: ImGui::PushMultiItemsWidths(COMPONENTS, ImGui::CalcItemWidth());

		for (uint32_t i = 0; i < COMPONENTS; i++)
		{
			ImGui::PushID(i);
			if (i > 0)
				ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
			value_changed |= ImGui::InputScalar("", ImGuiDataType_Float, p_data, nullptr, nullptr, FORMATS[i], flags);
			ImGui::PopID();
			ImGui::PopItemWidth();
			p_data = (void*)((char*)p_data + sizeof(float));
		}
		ImGui::PopID();

		const char* label_end = ImGui::FindRenderedTextEnd(LABEL);
		if (LABEL != label_end)
		{
			ImGui::SameLine(0.0f, g.Style.ItemInnerSpacing.x);
			ImGui::TextEx(LABEL, label_end);
		}

		ImGui::EndGroup();
		return value_changed;
	}

	bool	Splitter(						bool						split_vertically, 
											float						thickness, 
											float*						size1, 
											float*						size2, 
											float						min_size1, 
											float						min_size2, 
											float						splitter_long_axis_size)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiID id = window->GetID(split_vertically ? "##VSplitter" : "##HSplitter");

		ImRect	bb;
				bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
				bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);

		return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
	}

	void	TextCenter(						const std::string&			TEXT) 
	{
		const float FONT_SIZE = ImGui::GetFontSize() * TEXT.size() / 2.f;
		ImGui::SameLine( ImGui::GetWindowSize().x / 2.f - FONT_SIZE + (FONT_SIZE / 2.f));
		ImGui::Text(TEXT.c_str());
	}

	void	ItemTooltip(					const char*					TEXT,
											const float					MAX_WIDTH)
	{
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(glm::clamp(MAX_WIDTH, 32.f, 1024.f));
			ImGui::TextUnformatted(TEXT);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	bool	Checkbutton(					const char*					label, 
											bool*						bState,
											const ImVec2&				size_arg, 
											ImGuiButtonFlags			flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		const ImGuiStyle&	style		= GImGui->Style;
		const ImGuiID		id			= window->GetID(label);
		const ImVec2		label_size	= CalcTextSize(label, NULL, true);

		ImVec2 pos = window->DC.CursorPos;
		if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
			pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
		
		ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

		const ImRect bb(pos, pos + size);
		ItemSize(size, style.FramePadding.y);
		if (!ItemAdd(bb, id))
			return false;

		if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
			flags |= ImGuiButtonFlags_Repeat;
		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

		if (pressed)
		{
			*bState = !(*bState);
			MarkItemEdited(id);
		}

		// GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg)
		const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		RenderNavHighlight(bb, id);
		RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

		if(*bState)
		{
			window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_ButtonActive), 0.f, 0, 2.f);
		}
		
		RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

		return pressed;
	}

	bool	ColorEdit3(						const char*					LABEL, 
											glw::RGB&					color,
											ImGuiColorEditFlags			flags)
	{
		auto tmp = color.normalize();
		if(ColorEdit3(LABEL, &tmp[0], flags))
		{
			color = tmp;
			return true;
		}

		return false;
	}

	bool	ColorEdit4(						const char*					LABEL, 
											glw::RGBA&					color,
											ImGuiColorEditFlags			flags)
	{
		auto tmp = color.normalize();
		if(ColorEdit4(LABEL, &tmp[0], flags))
		{
			color = tmp;
			return true;
		}

		return false;
	}

	bool	Vec3Control(					glm::vec3&					value,
											const char*					LABEL, 
											const char*					ID,
											const char**				FORMAT,
											const ImGuiInputTextFlags_	FLAGS)
	{
		static const float  MAGIC_FACTOR    = 1.5f; // <- experimental value, works, leave it like that
		const ImVec2&       ITEM_SPACING    = ImGui::GetStyle().ItemSpacing;
		const float         WIDTH           = glm::max(0.f, ImGui::GetContentRegionAvailWidth() - ITEM_SPACING.x * MAGIC_FACTOR);
		
		ImGui::BeginGroupPanel(LABEL);
		bool bValueChanged = ImGui::PrefixedInputVector(ID, ImGuiDataType_Float, &value[0], 3, FORMAT, &WIDTH, FLAGS);  
		ImGui::EndGroupPanel();
		return bValueChanged;
	}

	void	Horizontal()
	{
		ImGui::GetCurrentWindow()->DC.LayoutType = ImGuiLayoutType_Horizontal;
	}

	void	Vertical(						const bool					bNEW_LINE)
	{
		auto& layout = ImGui::GetCurrentWindow()->DC.LayoutType;
		if(layout != ImGuiLayoutType_Vertical)
		{
			layout = ImGuiLayoutType_Vertical;
			if(bNEW_LINE) ImGui::NewLine();
		}
	}

	static ImVector<ImRect> s_GroupPanelLabelStack;

	ImRect	calculate_stretched_item_rect(	const ImVec2&				ITEM_SPACING,
											const float					FRAME_HEIGHT)
	{
		ImGui::EndGroup();
		ImGui::EndGroup();
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(FRAME_HEIGHT * 0.5f, 0.0f));
		ImGui::Dummy(ImVec2(0.0, FRAME_HEIGHT * 0.5f - ITEM_SPACING.y));
		ImGui::EndGroup();

		return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	ImRect	calculate_adjusted_item_rect(	const ImVec2&				ITEM_SPACING,
											const float					FRAME_HEIGHT)
	{
		ImGui::EndGroup();
		const ImVec2	ADJUSTED_MAX		= ImGui::GetItemRectMax();
		const float		HALF_FRAME_HEIGHT	= FRAME_HEIGHT * 0.5f;
		ImGui::EndGroup();
		ImGui::SameLine(0.0f, 0.0f);
		//ImGui::Dummy(ImVec2(FRAME_HEIGHT * 0.5f, 0.0f));
		//ImGui::Dummy(ImVec2(0.0, FRAME_HEIGHT * 0.5f - ITEM_SPACING.y));
		ImGui::Dummy(ImVec2(HALF_FRAME_HEIGHT, HALF_FRAME_HEIGHT - ITEM_SPACING.y));
		ImGui::EndGroup();

		return ImRect(ImGui::GetItemRectMin(), ADJUSTED_MAX + ImVec2(HALF_FRAME_HEIGHT, HALF_FRAME_HEIGHT - ITEM_SPACING.y));
	}

	void	BeginGroupPanel(				const char*					NAME, 
											const ImVec2&				SIZE)
	{
		ImGui::BeginGroup();

		auto itemWidth		= ImGui::CalcItemWidth();
		auto itemSpacing	= ImGui::GetStyle().ItemSpacing;
		
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	  
		ImGui::BeginGroup();

		auto frameHeight	= ImGui::GetFrameHeight();
		auto cursorPos		= ImGui::GetCursorScreenPos();

		ImVec2 effectiveSize = SIZE;
		if (SIZE.x < 0.0f)
			effectiveSize.x = ImGui::GetContentRegionAvailWidth();
		//else
		//    effectiveSize.x = SIZE.x;
		ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f));

		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::BeginGroup();
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::TextUnformatted(NAME);
		auto labelMin = ImGui::GetItemRectMin();
		auto labelMax = ImGui::GetItemRectMax();
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
		ImGui::BeginGroup();

		ImGui::PopStyleVar(2);

		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->WorkRect.Max.x          -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->InnerRect.Max.x         -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->Size.x                   -= frameHeight;

		//auto itemWidth = ImGui::CalcItemWidth();
		ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

		s_GroupPanelLabelStack.push_back(ImRect(labelMin, labelMax));
	}

	void	EndGroupPanel(					const bool					bSTRETCH)
	{
		ImGui::Dummy(ImVec2(0.f, 2.f));
		ImGui::PopItemWidth();

		auto itemSpacing	= ImGui::GetStyle().ItemSpacing;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		auto frameHeight	= ImGui::GetFrameHeight();
		auto itemRect		= bSTRETCH	? calculate_stretched_item_rect(itemSpacing, frameHeight)
										: calculate_adjusted_item_rect(itemSpacing, frameHeight);

		auto	labelRect = s_GroupPanelLabelStack.back();
				labelRect.Min.x -= itemSpacing.x;
				labelRect.Max.x += itemSpacing.x;

		s_GroupPanelLabelStack.pop_back();

		ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight) * 0.5f;
		ImRect frameRect = ImRect(itemRect.Min + halfFrame, itemRect.Max - ImVec2(halfFrame.x, 0.0f));
		
		for (int i = 0; i < 4; ++i)
		{
			switch (i)
			{
				// left half-plane
				case 0: ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(labelRect.Min.x, FLT_MAX), true); break;
				// right half-plane
				case 1: ImGui::PushClipRect(ImVec2(labelRect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true); break;
				// top
				case 2: ImGui::PushClipRect(ImVec2(labelRect.Min.x, -FLT_MAX), ImVec2(labelRect.Max.x, labelRect.Min.y), true); break;
				// bottom
				case 3: ImGui::PushClipRect(ImVec2(labelRect.Min.x, labelRect.Max.y), ImVec2(labelRect.Max.x, FLT_MAX), true); break;
			}

			ImGui::GetWindowDrawList()->AddRect(
				frameRect.Min, frameRect.Max,
				ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)),
				halfFrame.x);

			ImGui::PopClipRect();
		}

		ImGui::PopStyleVar(2);

		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->WorkRect.Max.x          += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->InnerRect.Max.x         += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->Size.x                  += frameHeight;

		ImGui::Dummy(ImVec2(0.0f, 0.0f));

		ImGui::EndGroup();
	}

	/*
	void BeginGroupPanel(const char* name, const ImVec2& size)
	{
		ImGui::BeginGroup();

		auto cursorPos = ImGui::GetCursorScreenPos();
		auto itemSpacing = ImGui::GetStyle().ItemSpacing;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		auto frameHeight = ImGui::GetFrameHeight();
		ImGui::BeginGroup();

		ImVec2 effectiveSize = size;
		if (size.x < 0.0f)
			effectiveSize.x = ImGui::GetContentRegionAvailWidth();
		else
			effectiveSize.x = size.x;
		ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f));

		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::BeginGroup();
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::TextUnformatted(name);
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
		ImGui::BeginGroup();

		ImGui::PopStyleVar(2);

		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x  -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->Size.x                   -= frameHeight;

		ImGui::PushItemWidth(effectiveSize.x - frameHeight);
	}

	void EndGroupPanel()
	{
		ImGui::PopItemWidth();

		auto itemSpacing = ImGui::GetStyle().ItemSpacing;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		auto frameHeight = ImGui::GetFrameHeight();

		ImGui::EndGroup();

		//ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(0, 255, 0, 64), 4.0f);

		ImGui::EndGroup();

		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

		ImGui::EndGroup();

		auto itemMin = ImGui::GetItemRectMin();
		auto itemMax = ImGui::GetItemRectMax();
		//ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(255, 0, 0, 64), 4.0f);

		ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight) * 0.5f;
		ImGui::GetWindowDrawList()->AddRect(
			itemMin + halfFrame, itemMax - ImVec2(halfFrame.x, 0.0f),
			ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)),
			halfFrame.x);

		ImGui::PopStyleVar(2);

		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x  += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->Size.x                   += frameHeight;

		ImGui::Dummy(ImVec2(0.0f, 0.0f));

		ImGui::EndGroup();
	}
	*/

	void	TooltipOverLastItem(			const char*					desc)
	{
		if (ImGui::IsItemHovered())
		{
			static const float WRAP_SCALE = 35.f;
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * WRAP_SCALE);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void	HelpMarker(						const char*					desc)
	{
		ImGui::TextDisabled("(?)");
		ImGui::TooltipOverLastItem(desc);
	}

	void	BufferingBar(					const char*					label, 
											float						value,  
											const ImVec2&				size_arg, 
											const ImU32&				bg_col, 
											const ImU32&				fg_col) 
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;
		
		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		ImVec2 pos = window->DC.CursorPos;
		ImVec2 size = size_arg;
		size.x -= style.FramePadding.x * 2;
		
		const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		ItemSize(bb, style.FramePadding.y);
		if (!ItemAdd(bb, id))
			return;
		
		// Render
		const float circleStart = size.x * 0.7f;
		const float circleEnd = size.x;
		const float circleWidth = circleEnd - circleStart;
		
		window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
		window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart*value, bb.Max.y), fg_col);
		
		const float t = (float)g.Time;
		const float r = size.y / 2;
		const float speed = 1.5f;
		
		const float a = speed*0;
		const float b = speed*0.333f;
		const float c = speed*0.666f;
		
		const float o1 = (circleWidth+r) * (t+a - speed * (int)((t+a) / speed)) / speed;
		const float o2 = (circleWidth+r) * (t+b - speed * (int)((t+b) / speed)) / speed;
		const float o3 = (circleWidth+r) * (t+c - speed * (int)((t+c) / speed)) / speed;
		
		window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, bg_col);
		window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, bg_col);
		window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, bg_col);
	}

	void	Spinner(						const char*					label, 
											float						radius, 
											int							thickness, 
											const ImU32&				color) 
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;
		
		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		
		ImVec2 pos = window->DC.CursorPos;
		ImVec2 size((radius )*2, (radius + style.FramePadding.y)*2);
		
		const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		ItemSize(bb, style.FramePadding.y);
		if (!ItemAdd(bb, id))
			return;
		
		// Render
		window->DrawList->PathClear();
		
		int num_segments = 30;
		int start = (int)abs(ImSin((float)g.Time*1.8f)*(num_segments-5));
		
		const float a_min = IM_PI*2.0f * ((float)start) / (float)num_segments;
		const float a_max = IM_PI*2.0f * ((float)num_segments-3) / (float)num_segments;

		const ImVec2 centre = ImVec2(pos.x+radius, pos.y+radius+style.FramePadding.y);
		
		for (int i = 0; i < num_segments; i++) {
			const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
			window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a+(float)g.Time*8) * radius,
												centre.y + ImSin(a+(float)g.Time*8) * radius));
		}

		window->DrawList->PathStroke(color, ImDrawFlags_::ImDrawFlags_None, (float)thickness);
	}  

	void	DotSpinner(						const char*					label, 
											const float					indicator_radius, 
											const ImU32&				main_color, 
											const ImU32&				backdrop_color, 
											const int					circle_count, 
											const float					speed) 
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems) 
			return;

		ImGuiContext& g = *GImGui;
		const ImGuiID id = window->GetID(label);

		
		const ImVec2 pos = window->DC.CursorPos;
		const ImRect bb(pos, ImVec2(pos.x + indicator_radius * 2.0f,
									pos.y + indicator_radius * 2.0f));
	  
		ItemSize(bb, g.Style.FramePadding.y);
		if (!ItemAdd(bb, id)) 
			return;

		const float t = (float)g.Time;
		const float circle_radius = indicator_radius / 10.0f;
		const float degree_offset = 2.0f * IM_PI / circle_count;

		ImColor color0(main_color);
		ImColor color1(backdrop_color);
			
		for (int i = 0; i < circle_count; ++i) 
		{
			const auto x = indicator_radius * std::sin(degree_offset * i);
			const auto y = indicator_radius * std::cos(degree_offset * i);

			const auto growth = glm::max(0.0f, std::sin(t * speed - i * degree_offset));

			ImVec4  final_color(  color0.Value.x * growth + color1.Value.x * (1.0f - growth)  // R
							   ,  color0.Value.y * growth + color1.Value.y * (1.0f - growth)  // G
							   ,  color0.Value.z * growth + color1.Value.z * (1.0f - growth)  // B
							   ,  1.f);                                                       // A

			window->DrawList->AddCircleFilled(ImVec2(pos.x + indicator_radius + x,
													 pos.y + indicator_radius - y),
													 circle_radius + growth * circle_radius,
													 GetColorU32(final_color));
		}
	}
}


static const char* PatchFormatStringFloatToInt2(const char* fmt)
{
	if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '0' && fmt[3] == 'f' && fmt[4] == 0) // Fast legacy path for "%.0f" which is expected to be the most common case.
		return "%d";
	const char* fmt_start = ImParseFormatFindStart(fmt);    // Find % (if any, and ignore %%)
	const char* fmt_end = ImParseFormatFindEnd(fmt_start);  // Find end of format specifier, which itself is an exercise of confidence/recklessness (because snprintf is dependent on libc or user).
	if (fmt_end > fmt_start && fmt_end[-1] == 'f')
	{
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
		if (fmt_start == fmt && fmt_end[0] == 0)
			return "%d";
		ImGuiContext& g = *GImGui;
		ImFormatString(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), "%.*s%%d%s", (int)(fmt_start - fmt), fmt, fmt_end); // Honor leading and trailing decorations, but lose alignment/precision.
		return g.TempBuffer;
#else
		IM_ASSERT(0 && "DragInt(): Invalid format string!"); // Old versions used a default parameter of "%.0f", please replace with e.g. "%d"
#endif
	}
	return fmt;
}


static const ImGuiDataTypeInfo GDataTypeInfo2[] =
{
	{ sizeof(char),             "%d",   "%d"    },  // ImGuiDataType_S8
	{ sizeof(unsigned char),    "%u",   "%u"    },
	{ sizeof(short),            "%d",   "%d"    },  // ImGuiDataType_S16
	{ sizeof(unsigned short),   "%u",   "%u"    },
	{ sizeof(int),              "%d",   "%d"    },  // ImGuiDataType_S32
	{ sizeof(unsigned int),     "%u",   "%u"    },
#ifdef _MSC_VER
	{ sizeof(ImS64),            "%I64d","%I64d" },  // ImGuiDataType_S64
	{ sizeof(ImU64),            "%I64u","%I64u" },
#else
	{ sizeof(ImS64),            "%lld", "%lld"  },  // ImGuiDataType_S64
	{ sizeof(ImU64),            "%llu", "%llu"  },
#endif
	{ sizeof(float),            "%f",   "%f"    },  // ImGuiDataType_Float (float are promoted to double in va_arg)
	{ sizeof(double),           "%f",   "%lf"   },  // ImGuiDataType_Double
};
IM_STATIC_ASSERT(IM_ARRAYSIZE(GDataTypeInfo2) == ImGuiDataType_COUNT);


bool ImGui::SpinScaler(const char* label, ImGuiDataType data_type, void* data_ptr, const void* step, const void* step_fast, const char* format, ImGuiInputTextFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiStyle& style = g.Style;

	if (format == NULL)
		format = DataTypeGetInfo(data_type)->PrintFmt;

	char buf[64];
	DataTypeFormatString(buf, IM_ARRAYSIZE(buf), data_type, data_ptr, format);

	bool value_changed = false;
	if ((flags & (ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsScientific)) == 0)
		flags |= ImGuiInputTextFlags_CharsDecimal;
	flags |= ImGuiInputTextFlags_AutoSelectAll;
	flags |= ImGuiInputTextFlags_NoMarkEdited;  // We call MarkItemEdited() ourselve by comparing the actual data rather than the string.

	if (step != NULL)
	{
		const float button_size = GetFrameHeight();

		BeginGroup(); // The only purpose of the group here is to allow the caller to query item data e.g. IsItemActive()
		PushID(label);
		SetNextItemWidth(ImMax(1.0f, CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2));
		if (InputText("", buf, IM_ARRAYSIZE(buf), flags)) // PushId(label) + "" gives us the expected ID from outside point of view
			value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, data_ptr, format);

		// Step buttons
		const ImVec2 backup_frame_padding = style.FramePadding;
		style.FramePadding.x = style.FramePadding.y;
		ImGuiButtonFlags button_flags = ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups;
		if (flags & ImGuiInputTextFlags_ReadOnly)
			button_flags |= ImGuiButtonFlags_Disabled;
		SameLine(0, style.ItemInnerSpacing.x);

// start diffs
		float frame_height = GetFrameHeight();
		float arrow_size = std::floor(frame_height * .45f);
		float arrow_spacing = frame_height - 2.0f * arrow_size;

		BeginGroup();
		PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{g.Style.ItemSpacing.x, arrow_spacing});

		// save/change font size to draw arrow buttons correctly
		float org_font_size = GetDrawListSharedData()->FontSize;
		GetDrawListSharedData()->FontSize = arrow_size;

		if (ArrowButtonEx("+", ImGuiDir_Up, ImVec2(arrow_size, arrow_size), button_flags))
		{
			DataTypeApplyOp(data_type, '+', data_ptr, data_ptr, g.IO.KeyCtrl && step_fast ? step_fast : step);
			value_changed = true;
		}

		if (ArrowButtonEx("-", ImGuiDir_Down, ImVec2(arrow_size, arrow_size), button_flags))
		{
			DataTypeApplyOp(data_type, '-', data_ptr, data_ptr, g.IO.KeyCtrl && step_fast ? step_fast : step);
			value_changed = true;
		}

		// restore font size
		GetDrawListSharedData()->FontSize = org_font_size;

		PopStyleVar(1);
		EndGroup();
// end diffs

		const char* label_end = FindRenderedTextEnd(label);
		if (label != label_end)
		{
			SameLine(0, style.ItemInnerSpacing.x);
			TextEx(label, label_end);
		}
		style.FramePadding = backup_frame_padding;

		PopID();
		EndGroup();
	}
	else
	{
		if (InputText(label, buf, IM_ARRAYSIZE(buf), flags))
			value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, data_ptr, format);
	}
	if (value_changed)
		MarkItemEdited(window->DC.LastItemId);

	return value_changed;
}

bool ImGui::SpinInt(const char* label, int* v, int step, int step_fast, ImGuiInputTextFlags flags)
{
	// Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
	const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
	return SpinScaler(label, ImGuiDataType_S32, (void*)v, (void*)(step>0 ? &step : NULL), (void*)(step_fast>0 ? &step_fast : NULL), format, flags);
}

bool ImGui::SpinFloat(const char* label, float* v, float step, float step_fast, const char* format, ImGuiInputTextFlags flags)
{
	flags |= ImGuiInputTextFlags_CharsScientific;
	return SpinScaler(label, ImGuiDataType_Float, (void*)v, (void*)(step>0.0f ? &step : NULL), (void*)(step_fast>0.0f ? &step_fast : NULL), format, flags);
}

bool ImGui::SpinDouble(const char* label, double* v, double step, double step_fast, const char* format, ImGuiInputTextFlags flags)
{
	flags |= ImGuiInputTextFlags_CharsScientific;
	return SpinScaler(label, ImGuiDataType_Double, (void*)v, (void*)(step>0.0 ? &step : NULL), (void*)(step_fast>0.0 ? &step_fast : NULL), format, flags);
}


bool ImGui::SliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, float power, float thickness)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const float w = CalcItemWidth();

	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	const ImVec2 ORIGINAL_SIZE = ImVec2(w, label_size.y + style.FramePadding.y * 2.0f);

	const float HALF_THICKNESS  = ORIGINAL_SIZE.y * glm::clamp(thickness, 0.f, 1.f) / 2.f;
	const float HALF_LOSS       = (ORIGINAL_SIZE.y / 2.f - HALF_THICKNESS);

	ImRect  BASE_FRAME(window->DC.CursorPos, window->DC.CursorPos + ORIGINAL_SIZE);
			BASE_FRAME.Min.y += HALF_THICKNESS;
			BASE_FRAME.Max.y += HALF_THICKNESS;

	ImRect  frame_bb = BASE_FRAME;
			frame_bb.Min.y += HALF_LOSS; // + HALF_THICKNESS;
			frame_bb.Max.y -= HALF_LOSS; // + HALF_THICKNESS;

	const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id, &frame_bb))
		return false;

	// Default format string when passing NULL
	if (format == NULL)
		format = DataTypeGetInfo(data_type)->PrintFmt;
	else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0) // (FIXME-LEGACY: Patch old "%.0f" format string to use "%d", read function more details.)
		format = PatchFormatStringFloatToInt2(format);

	// Tabbing or CTRL-clicking on Slider turns it into an input box
	const bool hovered = ItemHoverable(BASE_FRAME, id);
	bool temp_input_is_active = TempInputIsActive(id);
	bool temp_input_start = false;
	if (!temp_input_is_active)
	{
		const bool focus_requested = FocusableItemRegister(window, id);
		const bool clicked = (hovered && g.IO.MouseClicked[0]);
		if (focus_requested || clicked || g.NavActivateId == id || g.NavInputId == id)
		{
			SetActiveID(id, window);
			SetFocusID(id, window);
			FocusWindow(window);
			g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
			if (focus_requested || (clicked && g.IO.KeyCtrl) || g.NavInputId == id)
			{
				temp_input_start = true;
				FocusableItemUnregister(window);
			}
		}
	}

	// Our current specs do NOT clamp when using CTRL+Click manual input, but we should eventually add a flag for that..
	if (temp_input_is_active || temp_input_start)
		return TempInputScalar(frame_bb, id, label, data_type, p_data, format);// , p_min, p_max);

	// Draw frame
	const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
	RenderNavHighlight(frame_bb, id);
	RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

	// Slider behavior
	ImRect grab_bb;
	//const bool value_changed = SliderBehavior(BASE_FRAME, id, data_type, p_data, p_min, p_max, format, power, ImGuiSliderFlags_None, &grab_bb);
	const bool value_changed = SliderBehavior(BASE_FRAME, id, data_type, p_data, p_min, p_max, format, ImGuiSliderFlags_None, &grab_bb);
	if (value_changed)
		MarkItemEdited(id);

	// Correct grab thickness.
	//grab_bb.Min.y -= HALF_LOSS;
	//grab_bb.Max.y += HALF_LOSS;

	// Render grab
	if (grab_bb.Max.x > grab_bb.Min.x)
		window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);

	// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
	// Ignore text clipping.
	char value_buf[64];
	const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
	RenderTextClipped(BASE_FRAME.Min, BASE_FRAME.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f), &BASE_FRAME);

	if (label_size.x > 0.0f)
	{
		RenderText(ImVec2(BASE_FRAME.Max.x + style.ItemInnerSpacing.x, BASE_FRAME.Min.y + style.FramePadding.y), label);
	}

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
	return value_changed;
}

bool ImGui::SliderScalarN(const char* label, ImGuiDataType data_type, void* v, int components, const void* v_min, const void* v_max, const char* format, float power, float thickness)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	bool value_changed = false;
	BeginGroup();
	PushID(label);
	PushMultiItemsWidths(components, CalcItemWidth());
	size_t type_size = GDataTypeInfo2[data_type].Size;
	for (int i = 0; i < components; i++)
	{
		PushID(i);
		if (i > 0)
			SameLine(0, g.Style.ItemInnerSpacing.x);
		value_changed |= SliderScalar("", data_type, v, v_min, v_max, format, power, thickness);
		PopID();
		PopItemWidth();
		v = (void*)((char*)v + type_size);
	}
	PopID();

	const char* label_end = FindRenderedTextEnd(label);
	if (label != label_end)
	{
		SameLine(0, g.Style.ItemInnerSpacing.x);
		TextEx(label, label_end);
	}

	EndGroup();
	return value_changed;
}

bool ImGui::VSliderScalar(const char* label, const ImVec2& size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, float power, float thickness)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	const float HALF_LOSS = size.x * (1.f - glm::clamp(thickness, 0.f, 1.f)) / 2.f;

	ImRect  frame_bb(window->DC.CursorPos, window->DC.CursorPos);
			frame_bb.Max = frame_bb.Max + size;
			frame_bb.Min.x += HALF_LOSS;
			frame_bb.Max.x -= HALF_LOSS;

	const ImRect bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(frame_bb, id))
		return false;

	// Default format string when passing NULL
	if (format == NULL)
		format = DataTypeGetInfo(data_type)->PrintFmt;
	else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0) // (FIXME-LEGACY: Patch old "%.0f" format string to use "%d", read function more details.)
		format = PatchFormatStringFloatToInt2(format);

	const bool hovered = ItemHoverable(frame_bb, id);
	if ((hovered && g.IO.MouseClicked[0]) || g.NavActivateId == id || g.NavInputId == id)
	{
		SetActiveID(id, window);
		SetFocusID(id, window);
		FocusWindow(window);
		g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Up) | (1 << ImGuiDir_Down);
	}

	// Draw frame
	const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
	RenderNavHighlight(frame_bb, id);
	RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

	// Slider behavior
	ImRect grab_bb;
	//const bool value_changed = SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, power, ImGuiSliderFlags_Vertical, &grab_bb);
	const bool value_changed = SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, ImGuiSliderFlags_Vertical, &grab_bb);
	if (value_changed)
		MarkItemEdited(id);

	// Correct grab thickness.
	grab_bb.Min.x -= HALF_LOSS;
	grab_bb.Max.x += HALF_LOSS;

	// Render grab
	if (grab_bb.Max.y > grab_bb.Min.y)
		window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);

	// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
	// For the vertical slider we allow centered text to overlap the frame padding
	char value_buf[64];
	const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
	RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.0f));
	if (label_size.x > 0.0f)
		RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

	return value_changed;
}


bool ImGui::SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, float power, float thickness)
{
	return SliderScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, power, thickness);
}

bool ImGui::SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format, float power, float thickness)
{
	return SliderScalarN(label, ImGuiDataType_Float, v, 2, &v_min, &v_max, format, power, thickness);
}

bool ImGui::SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format, float power, float thickness)
{
	return SliderScalarN(label, ImGuiDataType_Float, v, 3, &v_min, &v_max, format, power, thickness);
}

bool ImGui::SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format, float power, float thickness)
{
	return SliderScalarN(label, ImGuiDataType_Float, v, 4, &v_min, &v_max, format, power, thickness);
}


bool ImGui::SliderAngle(const char* label, float* v_rad, float v_degrees_min, float v_degrees_max, const char* format, float thickness)
{
	if (format == NULL)
		format = "%.0f deg";
	float v_deg = (*v_rad) * 360.0f / (2 * IM_PI);
	bool value_changed = SliderFloat(label, &v_deg, v_degrees_min, v_degrees_max, format, 1.0f, thickness);
	*v_rad = v_deg * (2 * IM_PI) / 360.0f;
	return value_changed;
}


bool ImGui::SliderInt(const char* label, int* v, int v_min, int v_max, const char* format, float thickness)
{
	return SliderScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format, 1.f, thickness);
}

bool ImGui::SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format, float thickness)
{
	return SliderScalarN(label, ImGuiDataType_S32, v, 2, &v_min, &v_max, format, 1.f, thickness);
}

bool ImGui::SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format, float thickness)
{
	return SliderScalarN(label, ImGuiDataType_S32, v, 3, &v_min, &v_max, format, 1.f, thickness);
}

bool ImGui::SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format, float thickness)
{
	return SliderScalarN(label, ImGuiDataType_S32, v, 4, &v_min, &v_max, format, 1.f, thickness);
}


bool ImGui::VSliderFloat(const char* label, const ImVec2& size, float* v, float v_min, float v_max, const char* format, float power, float thickness)
{
	return VSliderScalar(label, size, ImGuiDataType_Float, v, &v_min, &v_max, format, power, thickness);
}

bool ImGui::VSliderInt(const char* label, const ImVec2& size, int* v, int v_min, int v_max, const char* format, float thickness)
{
	return VSliderScalar(label, size, ImGuiDataType_S32, v, &v_min, &v_max, format, 1.f, thickness);
}