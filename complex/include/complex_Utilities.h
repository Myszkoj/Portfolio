#pragma once

// std
#include <functional>
#include <string>
#include <sstream>

// dpl
#include <dpl_ReadOnly.h>
#include <dpl_Mask.h>
#include <dpl_TypeTraits.h>
#include <dpl_EventDispatcher.h>

// glw
#include <glw_Colors.h>
#include <glw_Utilities.h>
#pragma comment(lib, "glw.lib")

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#pragma warning( push )
#pragma warning( disable : 26495)
#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <ImGui/imgui_stdlib.h>
#include <ImGui/imgui_impl_sdl.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/implot.h>
#pragma warning( pop )


// SDL API
#pragma warning( push )
#pragma warning( disable : 26819)
#include <SDL2/SDL.h>		
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_events.h>
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma warning( pop )

// concepts
namespace complex
{
	template<typename T, uint32_t N>
	concept is_divisible_by = sizeof(T)%N == 0;
}

// functions
namespace complex
{
	inline auto			ss_size(				std::stringstream&				ss)
	{
		ss.seekp(0, std::ios::end);
		return ss.tellp();
	}

	inline void			replace_char(			const char						CURRENT,
												const char						NEW,
												std::string&					str)
	{
		std::replace_if(str.begin(), str.end(), [&](const char MARKER){return MARKER == CURRENT;}, NEW);
	}

	inline void			replace_char_with_word(	const char						CHAR,
												const std::string_view&			WORD,	
												std::string&					str)
	{
		size_t pos = std::string::npos;
		while ((pos = str.find('#')) != std::string::npos)
		{
			str.replace(pos, 1, WORD);
		}
	}

	std::string			get_windows_filter(		const std::string_view&			FILTER,
												const std::string_view&			EXT);

	std::string_view	get_file_folder(		const std::string_view&			FILE_PATH);

	std::string_view	get_file_name(			const std::string_view&			FILE_PATH);

	std::string_view	get_file_extension(		const std::string_view&			FILE_PATH);

	std::string			load_file(				const std::string&				FILE_PATH);
}