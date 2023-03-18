#pragma once


#include <string>
#include "complex_Utilities.h"
#include "complex_ImAddons.h"


namespace complex
{
	/*
		Interface for ImGui widgets.
	*/
	class Widget
	{
	public: // data
		dpl::ReadOnly<std::string,	Widget> name;
		dpl::ReadOnly<ImGuiID,		Widget> hash;			// ImGui hash ID
		dpl::ReadOnly<ImVec2,		Widget> requiredSize;	// Negative or zero values stretch widget to its parent's size.
		dpl::ReadOnly<bool,			Widget>	bVisible;

	public: // lifecycle
		CLASS_CTOR			Widget(				const std::string&		NAME,
												const ImVec2&			REQUIRED_SIZE = ImVec2(0.f, 0.f))
			: name(NAME)
			, hash(ImHashStr(NAME.c_str(), NAME.size()))
			, requiredSize(REQUIRED_SIZE)
			, bVisible(true)
		{
			
		}

		CLASS_CTOR			Widget(				const Widget&			OTHER) = delete;

		CLASS_CTOR			Widget(				Widget&&				other) noexcept = default;

		Widget&				operator=(			const Widget&			OTHER) = delete;

		Widget&				operator=(			Widget&&				other) noexcept = default;

		CLASS_DTOR virtual	~Widget() = default;

	public: // functions
		inline bool			is_visible() const
		{
			return bVisible;
		}

		inline void			toggle_visibility()
		{
			bVisible = !bVisible();
		}

		inline void			show()
		{
			bVisible = true;
		}

		inline void			hide()
		{
			bVisible = false;
		}

		inline ImVec2		get_size() const
		{
			const ImVec2 TMP = ImGui::GetContentRegionAvail();
			if(requiredSize().x > 0.f)
			{
				return (requiredSize().y > 0.f) ? requiredSize() : ImVec2(requiredSize().x, TMP.y);
			}
			else if(requiredSize().y > 0.f)
			{
				return ImVec2(TMP.x, requiredSize().y);
			}

			return TMP;
		}

		inline float		get_width() const
		{
			return requiredSize().x > 0.f ? requiredSize().x : ImGui::GetContentRegionAvailWidth();
		}

		inline void			stretch_H()
		{
			requiredSize->x = -1.f;
		}

		inline void			stretch_V()
		{
			requiredSize->y = -1.f;
		}

		inline void			stretch_HV()
		{
			stretch_H();
			stretch_V();
		}

		inline void			resize(				const float				NEW_WIDTH,
												const float				NEW_HEIGHT)
		{
			requiredSize->x = glm::clamp(NEW_WIDTH, 0.f, 8000.f);
			requiredSize->y = glm::clamp(NEW_HEIGHT, 0.f, 8000.f);
		}

		inline void			resize(				const ImVec2			NEW_SIZE)
		{
			requiredSize->x = glm::clamp(NEW_SIZE.x, 0.f, 8000.f);
			requiredSize->y = glm::clamp(NEW_SIZE.y, 0.f, 8000.f);
		}

		inline float		width() const
		{
			return requiredSize().x;
		}

		inline float		height() const
		{
			return requiredSize().y;
		}
	};
}