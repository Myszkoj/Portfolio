#include "..//include/complex_Utilities.h"


namespace complex
{
	std::string			get_windows_filter(		const std::string_view&			FILTER,
												const std::string_view&			EXT)
	{
		std::string str(FILTER);
		replace_char_with_word('#', EXT, str);
		replace_char('|', '\0', str);
		return str;
	}

	std::string_view	get_file_folder(		const std::string_view&			FILE_PATH)
	{
		if(FILE_PATH.size() > 0) 
		{
			const auto SEPARATOR_POSITION	= FILE_PATH.find_last_of("/\\");
			if(SEPARATOR_POSITION != std::numeric_limits<size_t>::max()-1)
			{
				return FILE_PATH.substr(0, SEPARATOR_POSITION +1);
			}
		}

		return "";
	}

	std::string_view	get_file_name(			const std::string_view&			FILE_PATH)
	{
		if(FILE_PATH.size() > 0)
		{
			/// crop path
			const auto	SEPARATOR_POSITION	= FILE_PATH.find_last_of("/\\");
			const auto	bHAS_FOLDER			= SEPARATOR_POSITION != std::numeric_limits<size_t>::max();
			const auto	NO_PATH				= bHAS_FOLDER? FILE_PATH.substr(SEPARATOR_POSITION + 1) : FILE_PATH;

			/// crop extension
			const auto DOT_POSITION			= NO_PATH.find_last_of(".");
			const auto HAS_EXTENSION		= DOT_POSITION != std::numeric_limits<size_t>::max();
			return NO_PATH.substr(0, DOT_POSITION);
		}

		return "";
	}

	std::string_view	get_file_extension(		const std::string_view&			FILE_PATH)
	{
		if(FILE_PATH.size() > 0)
		{
			const auto DOT_POSITION = FILE_PATH.find_last_of(".");
			if (DOT_POSITION != std::numeric_limits<size_t>::max())
			{
				return FILE_PATH.substr(DOT_POSITION);
			}
		}

		return "";
	}

	std::string			load_file(				const std::string&				FILE_PATH)
	{
		std::string result;

		std::ifstream stream(FILE_PATH.c_str());
		if(!stream.is_open())
			return result;

		stream.seekg(0, std::ios::end);
		result.reserve(stream.tellg());
		stream.seekg(0, std::ios::beg);

		result.assign((std::istreambuf_iterator<char>(stream)),
						std::istreambuf_iterator<char>());

		return result;
	}
}