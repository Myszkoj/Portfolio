#pragma once


#include <optional>
#include <functional>
#include <dpl_Values.h>
#include "complex_Sidebar.h"
#include "complex_Dockspace.h"
#include "complex_ChildWindow.h"


namespace complex
{
	class	Application;


	class	MainWindow
	{
	public: // relations
		friend	Application;

	public: // subtypes
		using	MenuBar			= std::function<void()>;

	public: // subtypes
		using	Position		= glm::tvec2<int32_t>;
		using	Size			= glm::tvec2<uint32_t>;
		using	Scale			= glm::vec2;
		using	Sidebars		= std::vector<std::unique_ptr<Sidebar>>;
		using	ChildWindows	= std::vector<std::unique_ptr<ChildWindow>>;
		using	Width			= dpl::RangedValue<uint32_t, 8, 8192>;
		using	Height			= dpl::RangedValue<uint32_t, 8, 8192>;

	public: // data
		dpl::ReadOnly<std::string,					MainWindow> name;
		dpl::ReadOnly<SDL_Window*,					MainWindow>	handle;
		dpl::ReadOnly<glw::RGBA,					MainWindow> background;
		dpl::ReadOnly<ImRect,						MainWindow> contentSpace;

	private: // data
		dpl::ReadOnly<MenuBar,						MainWindow> menuBar;
		dpl::ReadOnly<Sidebars,						MainWindow> sidebars;
		dpl::ReadOnly<std::unique_ptr<Dockspace>,	MainWindow> dockspace;
		dpl::ReadOnly<ChildWindows,					MainWindow> childWindows;
		std::unique_ptr<glw::Context>							context;

	public: // lifecycle
		CLASS_CTOR					MainWindow(						const std::string&			NAME);

		CLASS_CTOR					MainWindow(						const MainWindow&			other) = delete;

		CLASS_CTOR					MainWindow(						MainWindow&&				other) noexcept = delete;

		CLASS_DTOR					~MainWindow();

		MainWindow&					operator=(						const MainWindow&			OTHER) = delete;

		MainWindow&					operator=(						MainWindow&&				other) noexcept = delete;

	public: // functions
		void						enable_vsync();

		void						disable_vsync();

		bool						set_fullscreen(					const bool					bFLAG);

		void						set_minimum_size(				const Width					WIDTH,
																	const Height				HEIGHT);

		void						set_size(						const Width					WIDTH,
																	const Height				HEIGHT);

		void						set_cursor_position(			const int32_t				X,
																	const int32_t				Y) const;

		void						get_cursor_position(			int32_t&					x,
																	int32_t&					y) const;

		static void					get_global_cursor_position(		int32_t&					x,
																	int32_t&					y);

		static ImVec2				get_global_cursor_position();

		static void					set_relative_mouse_state(		const bool					bSTATE,
																	const bool					bPRESERVE_INITIAL_POSITION  = true);

		static bool					get_relative_mouse_state();

	public: // content functions
		inline void					set_MenuBar(					MenuBar						newMenuBar)
		{
			*menuBar = newMenuBar;
		}

		template<typename SidebarT, typename... CTOR>
		inline SidebarT*			add_sidebar(					CTOR&&...					args)
		{
			return static_cast<SidebarT*>(sidebars->emplace_back(std::make_unique<SidebarT>(std::forward<CTOR>(args)...)).get());
		}

		inline Dockspace*			add_dockspace()
		{
			dockspace = std::make_unique<Dockspace>();
			return dockspace().get();
		}

		template<typename WindowT, typename... CTOR>
		inline WindowT*				add_child_window(				CTOR&&...					args)
		{
			return static_cast<WindowT*>(childWindows->emplace_back(std::make_unique<WindowT>(std::forward<CTOR>(args)...)).get());
		}

	public: // queries
		/*
			Returns ID assigned by the OS.
		*/
		uint32_t					get_internalID() const;

		std::string					get_title() const;

		Position					get_position() const;

		Size						get_size() const;

		void*						get_win32_handle();

		std::optional<std::string>	openFileDialog(					const char*					FILE_FILTER,
																	const char*					DEFAULT_EXT);

		std::optional<std::string>	saveFileDialog(					const char*					FILE_FILTER,
																	const char*					DEFAULT_EXT,
																	const bool					bSHOW_OVERWRITE_PROMPT);
	
	private: // functions
		void						set_attributes();

		void						initialize_handle_and_context(	const std::string&			NAME);

		void						clear();

	private: // functions
		void						render_all();

		void						on_event(						const SDL_Event&			EVENT);

		void						update(							Application&				app);
	};
}