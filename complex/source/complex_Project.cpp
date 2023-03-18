#include "..//include/complex_Project.h"


namespace complex
{
	dpl::ReadOnly<std::string,	Project>	Project::ext = ".proj";

//=====> Project private: // functions
	bool		Project::set_path(	const std::string&	FILE_PATH,
									dpl::Logger&		logger,
									const OnSetPath&	FUNCTION)
	{
		const auto FILE_EXT = get_file_extension(FILE_PATH);
		if(FILE_EXT != Project::ext())
			return logger.push_error("Unknown project file extension: " + std::string(FILE_EXT));
		
		const auto FILE_NAME = get_file_name(FILE_PATH);
		if(FILE_NAME.size() == 0)
			return logger.push_error("Project file has no name.");
		
		const auto FILE_FOLDER = get_file_folder(FILE_PATH);
		// TODO: Check permission of the folder.

		const auto& OLD_NAME	= name();
		const auto& OLD_FOLDER	= folder();
		*name	= FILE_NAME;
		*folder	= FILE_FOLDER;
		if(!FUNCTION(FILE_PATH))
		{
			name	= OLD_NAME;
			folder	= OLD_FOLDER;
		}

		return true;
	}

	void		Project::close()
	{
		// TODO: Destroy all components here.
		flags->clear();
		*name	= "";
		*folder	= "";
	}
}