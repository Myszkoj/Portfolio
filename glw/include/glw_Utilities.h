#pragma once

// std
#include <functional>
#include <string>
#include <fstream>
#include <unordered_map>
#include <typeindex>
#include <array>
#include <vector>

// dpl
#include <dpl_ReadOnly.h>
#include <dpl_GeneralException.h>
#include <dpl_Range.h>
#include <dpl_Mask.h>
#include <dpl_TypeLock.h>
#include <dpl_Subject.h>
#include <dpl_Compendium.h>
#include <dpl_Binary.h>
#include <dpl_Singleton.h>
#include <dpl_Variation.h>
#include <dpl_ResourceControl.h>

// cml
#include <cml.h>
#pragma comment(lib, "cml.lib")

// GL extension wrangler
#include <GL/glew.h> // note that it has to be included before SDL
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glew32.lib")

// DevIL image loader
#include <IL/il.h>  
#include <IL/ilu.h>
#pragma comment(lib, "DevIL.lib")
#pragma comment(lib, "ilu.lib")

namespace glw
{
	class	Context : public dpl::Singleton<Context>
	{
	public: // subtypes
		struct	Version
		{
			uint32_t major = 0;
			uint32_t minor = 0;
		};

		/*
			Protects opengl resources against simultaneous access from multiple threads.
		*/
		struct	Lock : public dpl::TypeLock<Lock>{};

		using	GLHandle	= void*;
		using	Initialize	= std::function<GLHandle(void* windowHandle)>;
		using	Release		= std::function<void(GLHandle glHandle)>;

	public: // data
		dpl::ReadOnly<void*,	Context> handle;
		dpl::ReadOnly<Version,	Context> version;

	private: // data
		Release OnRelease;

	public: // lifecycle
		CLASS_CTOR		Context(		void*				windowHandle,
										const Initialize	INITIALIZE,
										const Release		RELEASE);

		CLASS_CTOR		Context(		const Context&		OTHER) = delete;

		CLASS_CTOR		Context(		Context&&			other) noexcept = delete;

		CLASS_DTOR		~Context();

		Context&		operator=(		const Context&		OTHER) = delete;

		Context&		operator=(		Context&&			other) noexcept = delete;

	public: // functions
		std::string		get_info() const;
	};


	class	GLException : public dpl::GeneralException
	{
	public:
		CLASS_CTOR	GLException(	const std::string&	MESSAGE,
									const GLenum		ERROR_CODE)
			: GeneralException(error_message(MESSAGE) + opengl_message(ERROR_CODE) + "\n")

		{
			
		}

		CLASS_CTOR	GLException(	const int			LINE, 
									const std::string&	MESSAGE,
									const GLenum		ERROR_CODE)
			: GeneralException(LINE, error_message(MESSAGE) + opengl_message(ERROR_CODE) + "\n")

		{
			
		}

		CLASS_CTOR	GLException(	const char*			FILE, 
									const int			LINE, 
									const std::string&	MESSAGE,
									const GLenum		ERROR_CODE)
			: GeneralException(FILE, LINE, error_message(MESSAGE) + opengl_message(ERROR_CODE) + "\n")

		{
			
		}

	private:
		std::string opengl_message(	const GLenum		ERROR_CODE);
	};


	inline void			crash(					const std::string_view	ERROR_MESSAGE)
	{
		std::ofstream log("glw_crash");
		log << ERROR_MESSAGE;
		log.close();
		std::exit(999);
	}

	inline void			validate_opengl(		const char*				ERROR_MESSAGE)
	{
#ifdef _DEBUG
		if (GLenum error = glGetError())
			throw GLException(ERROR_MESSAGE, error);
#endif // _DEBUG
	}

	inline void			validate_opengl(		const int				LINE,
												const char*				ERROR_MESSAGE)
	{
#ifdef _DEBUG
		if (GLenum error = glGetError())
			throw GLException(LINE, ERROR_MESSAGE, error);
#endif // _DEBUG
	}

	inline void			validate_opengl(		const char*				FILE,
												const int				LINE,
												const char*				ERROR_MESSAGE)
	{
#ifdef _DEBUG
		if (GLenum error = glGetError())
			throw GLException(FILE, LINE, ERROR_MESSAGE, error);
#endif // _DEBUG
	}

	inline void			validate_opengl(		const char*				FILE,
												const int				LINE,
												const std::string&		ERROR_MESSAGE)
	{
		validate_opengl(FILE, LINE, ERROR_MESSAGE.c_str());
	}

	

	bool				isDepth(				const GLenum			FORMAT);

	bool				isStencil(				const GLenum			FORMAT);

	bool				isUNormColor(			const GLenum			FORMAT);

	bool				isSNormColor(			const GLenum			FORMAT);

	bool				isFloatColor(			const GLenum			FORMAT);

	bool				isSignedIntColor(		const GLenum			FORMAT);

	bool				isUnsignedIntColor(		const GLenum			FORMAT);
}

// OBSOLETE
namespace glw
{
	inline void			import_string(			std::istream&			binary,
												std::string&			string)
	{
		dpl::import_dynamic_container(binary, string);
	}

	inline std::string	import_string(			std::istream&			binary)
	{
		std::string string;
		import_string(binary, string);
		return string;
	}

	inline void			export_string(			std::ostream&			binary,
												const std::string&		STRING)
	{
		dpl::export_container(binary, STRING);
	}
}