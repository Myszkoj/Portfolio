#pragma once


#include <optional>
#include "complex_Widget.h"


namespace complex
{
	class Window : public Widget
	{
	public: // subtypes
		using MenuBar = std::function<void(Window& thisWindow)>;
		using Content = std::function<void(Window& thisWindow)>;

	public: // data
		std::optional<ImVec2>	WindowPadding;    // Padding within a window.
		std::optional<ImVec2>	FramePadding;     // Padding within a framed rectangle (used by most widgets).
		std::optional<ImVec2>	ItemSpacing;      // Horizontal and vertical spacing between widgets/lines.
		std::optional<ImVec2>	ItemInnerSpacing; // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label).

	protected: // data
		dpl::ReadOnly<ImGuiWindowFlags,	Window> flags;
		dpl::ReadOnly<MenuBar,			Window> menuBar;
		dpl::ReadOnly<Content,			Window> content;

	protected: // lifecycle
		CLASS_CTOR			Window(							const std::string&			NAME,
															const ImVec2&				REQUIRED_SIZE = ImVec2(-1.f, -1.f))
			: Widget(NAME, REQUIRED_SIZE)
			, flags(ImGuiWindowFlags_None)
		{

		}

		CLASS_CTOR			Window(							const Window&				OTHER) = delete;

		CLASS_CTOR			Window(							Window&&					other) noexcept = default;

		Window&				operator=(						const Window&				OTHER) = delete;

		Window&				operator=(						Window&&					other) noexcept = default;

	protected: // functions
		/*
			Sets window menu-bar(can be nullptr).
		*/
		inline void			set_MenuBar(					MenuBar						newMenuBar)
		{
			menuBar = newMenuBar;
			set_flag(ImGuiWindowFlags_MenuBar, menuBar() ? true : false);
		}

		/*
			Sets content of the window.
		*/
		inline void			set_Content(					Content						newContent)
		{
			content = newContent;
		}

		/*
			Disable title-bar.
		*/
		inline void			set_NoTitleBar(					const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_NoTitleBar, bVALUE);
		}

		/*
			Disable user resizing with the lower-right grip.
		*/
		inline void			set_NoResize(					const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_NoResize, bVALUE);
		}

		/*
			Disable user moving the window.
		*/
		inline void			set_NoMove(						const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_NoMove, bVALUE);
		}

		/*
			Disable scrollbars (window can still scroll with mouse or programmatically).
		*/
		inline void			set_NoScrollbar(				const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_NoScrollbar, bVALUE);
		}

		/*
			Disable user vertically scrolling with mouse wheel. 
			On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
		*/
		inline void			set_NoScrollWithMouse(			const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_NoScrollWithMouse, bVALUE);
		}

		/*
			Disable user collapsing window by double-clicking on it. 
			Also referred to as "window menu button" within a docking node.
		*/
		inline void			set_NoCollapse(					const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_NoCollapse, bVALUE);
		}

		/*
			Resize every window to its content every frame.
		*/
		inline void			set_AlwaysAutoResize(			const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_AlwaysAutoResize, bVALUE);
		}

		/*
			Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
		*/
		inline void			set_NoBackground(				const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_NoBackground, bVALUE);
		}

		/*
			Never load/save settings in .ini file.
		*/
		inline void			set_NoSavedSettings(			const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_NoSavedSettings, bVALUE);
		}

		/*
			Disable catching mouse, hovering test with pass through.
		*/
		inline void			set_NoMouseInputs(				const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_NoMouseInputs, bVALUE);
		}

		/*
			Allow horizontal scrollbar to appear (off by default). 
			You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. 
			Read code in imgui_demo in the "Horizontal Scrolling" section.
		*/
		inline void			set_HorizontalScrollbar(		const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_HorizontalScrollbar, bVALUE);
		}

		/*
			Disable taking focus when transitioning from hidden to visible state.
		*/
		inline void			set_NoFocusOnAppearing(			const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_NoFocusOnAppearing, bVALUE);
		}

		/*
			Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus).
		*/
		inline void			set_NoBringToFrontOnFocus(		const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_NoBringToFrontOnFocus, bVALUE);
		}

		/*
			Always show vertical scrollbar (even if ContentSize.y < Size.y).
		*/
		inline void			set_AlwaysVerticalScrollbar(	const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_AlwaysVerticalScrollbar, bVALUE);
		}

		/*
			Always show horizontal scrollbar (even if ContentSize.x < Size.x).
		*/
		inline void			set_AlwaysHorizontalScrollbar(	const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_AlwaysHorizontalScrollbar, bVALUE);
		}

		/*
			Ensure child windows without border uses style.WindowPadding (ignored by default for non-bordered child windows, because more convenient).
		*/
		inline void			set_AlwaysUseWindowPadding(		const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_AlwaysUseWindowPadding, bVALUE);
		}

		/*
			No gamepad/keyboard navigation within the window.
		*/
		inline void			set_NoNavInputs(				const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_NoNavInputs, bVALUE);
		}

		/*
			No focusing toward this window with gamepad/keyboard navigation (e.g. skipped by CTRL+TAB).
		*/
		inline void			set_NoNavFocus(					const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_NoNavFocus, bVALUE);
		}

		/*
			Append '*' to title without affecting the ID, as a convenience to avoid using the ### operator. 
			When used in a tab/docking context, tab is selected on closure and closure is deferred by one frame 
			to allow code to cancel the closure (with a confirmation popup, etc.) without flicker.
		*/
		inline void			set_UnsavedDocument(			const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_UnsavedDocument, bVALUE);
		}

		/*
			Disable docking of this window.
		*/
		inline void			set_NoDocking(					const bool					bVALUE)
		{
			set_flag(ImGuiWindowFlags_NoDocking, bVALUE);
		}

	private: // functions
		inline void			set_flag(						const ImGuiWindowFlags_		FLAG,
															const bool					bVALUE)
		{
			bVALUE	? *flags |= FLAG
					: *flags &= ~FLAG;
		}

		inline bool			get_flag(						const ImGuiWindowFlags_		FLAG) const
		{
			return flags() & FLAG;
		}

	public: // interface
		virtual void		open()
		{
			show();
		}

		virtual void		close()
		{
			hide();
		}

		virtual void		update();
	};
}