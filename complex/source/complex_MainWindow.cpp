#include "..//include/complex_MainWindow.h"
#include <glw_FrameBuffer.h>

#pragma warning( push )
#pragma warning( disable : 26819)
#include <SDL2\SDL_syswm.h>
#pragma warning( pop )

#include <commdlg.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#include <Windows.h>

namespace complex
{
	HRESULT set_NCRendering(HWND hWnd, DWMNCRENDERINGPOLICY ncrp)
	{
		HRESULT hr = S_OK;

		// Disable non-client area rendering on the window.
		hr = ::DwmSetWindowAttribute(hWnd,
			DWMWA_NCRENDERING_POLICY,
			&ncrp,
			sizeof(ncrp));

		if (SUCCEEDED(hr))
		{
			int breakpoint = 0;
		}

		return hr;
	}

	// Those values are used to fix invalid mouse delta in ImGui
	static bool		bRelativeMouse		= false;
	static int32_t	initialX			= -1;
	static int32_t	initialY			= -1;
	static int		deltaCorrectionX	= 0;
	static int		deltaCorrectionY	= 0;

//=====> MainWindow public: // lifecycle
	CLASS_CTOR					MainWindow::MainWindow(						const std::string&			NAME)
		: name(NAME)
		, handle(nullptr)
	{
		set_attributes();
		initialize_handle_and_context(NAME);
	}

	CLASS_DTOR					MainWindow::~MainWindow()
	{
		if(handle)
		{
			*menuBar = nullptr;
			sidebars->clear();
			dockspace->reset();
			childWindows->clear();
			context.reset();
			SDL_DestroyWindow(handle);
			handle = nullptr;
		}
	}

//=====> MainWindow public: // functions
	void						MainWindow::enable_vsync()
	{
		SDL_GL_SetSwapInterval(1);
	}

	void						MainWindow::disable_vsync()
	{
		SDL_GL_SetSwapInterval(0);
	}

	bool						MainWindow::set_fullscreen(					const bool					bFLAG)
	{
		if(bFLAG == true)
		{
			if (SDL_SetWindowFullscreen(handle, SDL_TRUE) < 0)
				return false;
		}
		else
		{
			if (SDL_SetWindowFullscreen(handle, SDL_FALSE) < 0)
				return false;
		}

		return true;
	}

	void						MainWindow::set_minimum_size(				const Width					WIDTH,
																			const Height				HEIGHT)
	{
		SDL_SetWindowMinimumSize(handle, WIDTH, HEIGHT);
	}

	void						MainWindow::set_size(						const Width					WIDTH,
																			const Height				HEIGHT)
	{
		SDL_SetWindowSize(handle, WIDTH, HEIGHT);
	}

	void						MainWindow::set_cursor_position(			const int32_t				X,
																			const int32_t				Y) const
	{
		SDL_WarpMouseInWindow(handle, X, Y);
	}

	void						MainWindow::get_cursor_position(			int32_t&					x,
																			int32_t&					y) const
	{
		SDL_GetMouseState(&x, &y);
	}

	void						MainWindow::get_global_cursor_position(		int32_t&					x,
																			int32_t&					y)
	{
		SDL_GetGlobalMouseState(&x, &y);
	}

	ImVec2						MainWindow::get_global_cursor_position()
	{
		int mouseX = 0;
		int mouseY = 0;
		SDL_GetGlobalMouseState(&mouseX, &mouseY);
		return ImVec2(float(mouseX), float(mouseY));
	}

	void						MainWindow::set_relative_mouse_state(		const bool					bSTATE,
																			const bool					bPRESERVE_INITIAL_POSITION)
	{
		if(bRelativeMouse != bSTATE)
		{
			if(bSTATE)
			{
				SDL_GetGlobalMouseState(&initialX, &initialY);
				SDL_SetRelativeMouseMode(SDL_TRUE);
			}
			else
			{
				SDL_SetRelativeMouseMode(SDL_FALSE);
				if(bPRESERVE_INITIAL_POSITION)
				{
					SDL_WarpMouseGlobal(initialX, initialY);
				}

				initialX = 0;
				initialY = 0;
			}

			bRelativeMouse = bSTATE;
		}
	}

	bool						MainWindow::get_relative_mouse_state()
	{
		return bRelativeMouse;
	}

//=====> MainWindow public: // queries
	uint32_t					MainWindow::get_internalID() const
	{
		return SDL_GetWindowID(handle);
	}

	std::string					MainWindow::get_title() const
	{
		return std::string(SDL_GetWindowTitle(handle));
	}

	MainWindow::Position		MainWindow::get_position() const
	{
		int32_t x = 0;
		int32_t y = 0;
		SDL_GetWindowPosition(handle, &x, &y);

		return Position(x, y);
	}

	MainWindow::Size			MainWindow::get_size() const
	{
		int width	= 0;
		int height	= 0;
		SDL_GetWindowSize(handle, &width, &height);
		return Size(width, height);
	}

	void*						MainWindow::get_win32_handle()
	{
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);

		if(SDL_GetWindowWMInfo(handle, &info)) 
		{
			const char* subsystem = "an unknown system!";
			switch(info.subsystem) 
			{
			case SDL_SYSWM_UNKNOWN:   break;
			case SDL_SYSWM_WINDOWS:   subsystem = "Microsoft Windows(TM)";  break;
			case SDL_SYSWM_X11:       subsystem = "X MainWindow System";    break;
#if SDL_VERSION_ATLEAST(2, 0, 3)
			case SDL_SYSWM_WINRT:     subsystem = "WinRT";                  break;
#endif
			case SDL_SYSWM_DIRECTFB:  subsystem = "DirectFB";               break;
			case SDL_SYSWM_COCOA:     subsystem = "Apple OS X";             break;
			case SDL_SYSWM_UIKIT:     subsystem = "UIKit";                  break;
#if SDL_VERSION_ATLEAST(2, 0, 2)
			case SDL_SYSWM_WAYLAND:   subsystem = "Wayland";                break;
			case SDL_SYSWM_MIR:       subsystem = "Mir";                    break;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 4)
			case SDL_SYSWM_ANDROID:   subsystem = "Android";                break;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 5)
			case SDL_SYSWM_VIVANTE:   subsystem = "Vivante";                break;
#endif
			}

			SDL_Log("This program is running SDL version %d.%d.%d on %s",
					(int)info.version.major,
					(int)info.version.minor,
					(int)info.version.patch,
					subsystem);
		} 
		else 
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't get window information: %s", SDL_GetError());
		}

		return info.info.win.window;
	}

	std::optional<std::string>	MainWindow::openFileDialog(					const char*					FILE_FILTER,
																			const char*					DEFAULT_EXT)
	{
		static const uint32_t MAX_FILE_NAME_CHARACTERS	= 2048;
		std::vector<char> fileNameBuffer(MAX_FILE_NAME_CHARACTERS, 0);

		OPENFILENAME	dialogBox; memset(&dialogBox, 0, sizeof(dialogBox));
						dialogBox.lStructSize	= sizeof(dialogBox); // SEE NOTE BELOW
						dialogBox.hwndOwner		= (HWND)get_win32_handle();
						dialogBox.lpstrFilter	= FILE_FILTER; //"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
						dialogBox.lpstrFile		= fileNameBuffer.data();
						dialogBox.nMaxFile		= MAX_PATH;
						dialogBox.Flags			= OFN_NOCHANGEDIR | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_NOREADONLYRETURN; // | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
						dialogBox.lpstrDefExt	= DEFAULT_EXT;

		if(GetOpenFileName(&dialogBox))
		{
			// https://stackoverflow.com/questions/26317556/how-to-get-list-of-selected-files-when-using-getopenfilename-with-multiselect
			// TODO: Add support for multiselection of the files.
			auto it = std::find(fileNameBuffer.begin(), fileNameBuffer.end(), '\0');
			if(it != fileNameBuffer.end())
			{
				return std::string(fileNameBuffer.begin(), it);
			}
		}

		return std::nullopt;
	}

	std::optional<std::string>	MainWindow::saveFileDialog(					const char*					FILE_FILTER,
																			const char*					DEFAULT_EXT,
																			const bool					bSHOW_OVERWRITE_PROMPT)
	{
		static const uint32_t MAX_FILE_NAME_CHARACTERS	= 2048;
		std::vector<char> fileNameBuffer(MAX_FILE_NAME_CHARACTERS, 0);

		OPENFILENAME	dialogBox; memset(&dialogBox, 0, sizeof(dialogBox));
						dialogBox.lStructSize	= sizeof(dialogBox); // SEE NOTE BELOW
						dialogBox.hwndOwner		= (HWND)get_win32_handle();
						dialogBox.lpstrFilter	= FILE_FILTER; //"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
						dialogBox.lpstrFile		= fileNameBuffer.data();
						dialogBox.nMaxFile		= MAX_PATH;
						dialogBox.Flags			= OFN_NOCHANGEDIR | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_NOREADONLYRETURN; // | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
						dialogBox.lpstrDefExt	= DEFAULT_EXT;

		if(bSHOW_OVERWRITE_PROMPT)
		{
			dialogBox.Flags |= OFN_OVERWRITEPROMPT;
		}

		if(GetSaveFileName(&dialogBox))
		{
			auto it = std::find(fileNameBuffer.begin(), fileNameBuffer.end(), '\0');
			if(it != fileNameBuffer.end())
			{
				return std::string(fileNameBuffer.begin(), it);
			}
		}

		return std::nullopt;
	}

//=====> MainWindow private: // functions
	void						MainWindow::set_attributes()
	{
		if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) > 0)
			throw dpl::GeneralException(this, __LINE__, SDL_GetError());

		if (SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8) > 0)
			throw dpl::GeneralException(this, __LINE__, SDL_GetError());
		if (SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8) > 0)
			throw dpl::GeneralException(this, __LINE__, SDL_GetError());
		if (SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8) > 0)
			throw dpl::GeneralException(this, __LINE__, SDL_GetError());
		if (SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8) > 0)
			throw dpl::GeneralException(this, __LINE__, SDL_GetError());

		if (SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0) > 0)
			throw dpl::GeneralException(this, __LINE__, SDL_GetError());
	}

	void						MainWindow::initialize_handle_and_context(	const std::string&			NAME)
	{
		SDL_DisplayMode displayMode;
		SDL_GetCurrentDisplayMode(0, &displayMode);

		const int		INITIAL_WIDTH	= displayMode.w - 80;
		const int		INITIAL_HEIGHT	= displayMode.h - 80;

		const Uint32	FLAGS = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
	
		handle = SDL_CreateWindow(NAME.c_str(), 0, 0, INITIAL_WIDTH, INITIAL_HEIGHT, FLAGS);
		if (!handle)
			throw dpl::GeneralException(this, __LINE__, std::string("Fail to create SDL_Window: ") + SDL_GetError());

		SDL_SetWindowPosition(handle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		if(!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
		{
			const auto		ERROR_CODE		= GetLastError();
			LPSTR messageBuffer = nullptr;

			const size_t	ERROR_STR_SIZE	= FormatMessageA(	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
																NULL, ERROR_CODE, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
	
			const std::string ERROR_MESSAGE(messageBuffer, ERROR_STR_SIZE);
			LocalFree(messageBuffer);

			throw dpl::GeneralException(this, __LINE__, "Fail to set priority class: " + ERROR_MESSAGE);
		}
			

		if(!context)
		{
			context = std::make_unique<glw::Context>(	handle(), 
														[](void* windowHandle){return SDL_GL_CreateContext((SDL_Window*)windowHandle);}, 
														[](void* glContext){SDL_GL_DeleteContext(glContext);});
		}
	}

	void						MainWindow::clear()
	{
		glw::FrameBuffer::bind_default(glw::FrameBuffer::Mode::eWRITE);

		auto size	= get_size();
		auto color	= background().normalize();

		glViewport(0, 0, size.x, size.y);
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

//=====> MainWindow private: // functions
	void						MainWindow::render_all()
	{
		// Perform internal rendering of the GUI.
		ImGui::Render();

		// Render new frame to the window.
		MainWindow::clear();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			SDL_Window*		backup_current_window	= SDL_GL_GetCurrentWindow();
			SDL_GLContext	backup_current_context	= SDL_GL_GetCurrentContext();

			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();

			SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
		}

		// Display current frame on the screen.
		SDL_GL_SwapWindow(handle);
	}

	void						MainWindow::on_event(						const SDL_Event&			EVENT)
	{
		//const auto& MY_ID = SDL_GetWindowID(handle());

		switch(EVENT.type)
		{
		//case SDL_KEYDOWN:
		//case SDL_KEYUP:
		//case SDL_MOUSEBUTTONDOWN:
		//case SDL_MOUSEBUTTONUP:
		//	break;

		case SDL_MOUSEMOTION:
			// Store sum of all deltas.
			deltaCorrectionX += EVENT.motion.xrel;
			deltaCorrectionY += EVENT.motion.yrel;
			break;

		//case SDL_MOUSEWHEEL:
		//	break;

		default: break;
		}
	}

	void						MainWindow::update(							Application&				app)
	{
		// W razie problemu z renderowaniem poza g³ównym oknem:  https://gitmemory.com/issue/ocornut/imgui/2600/517429173

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(handle);
		ImGui::NewFrame();

		{ // Fix mouse delta(must be done after ImGui::NewFrame).
			auto&	delta = ImGui::GetIO().MouseDelta;
					delta.x = static_cast<float>(deltaCorrectionX);
					delta.y = static_cast<float>(deltaCorrectionY);

			deltaCorrectionX = 0;
			deltaCorrectionY = 0;
		}
		
		if(ImGuiViewport* main_viewport = ImGui::GetMainViewport())
		{
			if(menuBar())
			{
				if(ImGui::BeginMainMenuBar())
				{
					menuBar()();
					ImGui::EndMainMenuBar();
				}
			}

			contentSpace->Min = main_viewport->WorkPos;
			contentSpace->Max = main_viewport->WorkPos + main_viewport->WorkSize;

			if(dockspace())
			{
				dockspace()->set_offset(contentSpace().Min);
				dockspace()->resize(contentSpace().GetSize());
				dockspace()->update();
			}

			for(auto& it : sidebars())
			{
				it->update(main_viewport);
			}

			for(auto& it : childWindows())
			{
				it->update();
			}
		}

		ImGui::EndFrame(); 

		render_all();
	}
}