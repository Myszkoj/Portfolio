#pragma once


#include "complex_Window.h"


namespace complex
{
	/*
		Window that can be collapsed to a titlebar.
	*/
	class ContentWindow : public Window
	{
	public: // data
		dpl::ReadOnly<float,				ContentWindow> rounding;
		dpl::ReadOnly<ImDrawCornerFlags,	ContentWindow> cornerFlags;
		dpl::ReadOnly<bool,					ContentWindow> bHasBorder;
		dpl::ReadOnly<bool,					ContentWindow> bCollapsed;

	private: // data
		dpl::ReadOnly<ImGuiID,				ContentWindow> headerFrameHash;
		dpl::ReadOnly<ImGuiID,				ContentWindow> headerContentHash;

	private: // lifecycle
		CLASS_CTOR			ContentWindow(		const std::string&			NAME,
												const std::string			HEADER_FRAME_STR,
												const std::string			HEADER_CONTENT_STR,
												const ImVec2&				REQUIRED_SIZE,
												const bool					bHAS_BORDER);

	public: // lifecycle
		CLASS_CTOR			ContentWindow(		const std::string&			NAME,
												const ImVec2&				REQUIRED_SIZE	= ImVec2(-1.f, -1.f),
												const bool					bHAS_BORDER		= true);

		CLASS_CTOR			ContentWindow(		const ContentWindow&		OTHER) = delete;

		CLASS_CTOR			ContentWindow(		ContentWindow&&				other) noexcept = default;

		ContentWindow&		operator=(			const ContentWindow&		OTHER) = delete;

		ContentWindow&		operator=(			ContentWindow&&				other) noexcept = default;

	protected: // functions
		inline void			set_corner_flag(	const ImDrawCornerFlags_	FLAG,
												const bool					bVALUE)
		{
			bVALUE	? *cornerFlags |= FLAG
					: *cornerFlags &= ~FLAG;
		}

		inline bool			get_corner_flag(	const ImDrawCornerFlags_	FLAG) const
		{
			return cornerFlags() & FLAG;
		}

	private: // functions
		bool                begin_collapsing();

		void                render_frame_content();

		void                end_collapsing();

	public: // implementation
		using Window::set_Content;

		virtual void		update() final override;
	};
}