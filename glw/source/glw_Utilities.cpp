#include "..//include/glw_Utilities.h"


namespace glw
{
	CLASS_CTOR		Context::Context(		void*				windowHandle,
											const Initialize	INITIALIZE,
											const Release		RELEASE)
		: handle(nullptr)
		, OnRelease(RELEASE)
	{
		auto initialize_context = [&](const Version VERSION)
		{
			// create OpenGL context
			if (this->handle = INITIALIZE(windowHandle))
			{
				////// GLEW //////	
				glewExperimental = GL_TRUE;
				GLenum glewError = glewInit();
				if (glewError != GLEW_OK)
					throw dpl::GeneralException(this, __LINE__, std::string("Fail to initialize GLEW.") + reinterpret_cast<const char*>(glewGetErrorString(glewError)));

				version = VERSION;
				return true;
			}

			return false;
		};

		// try OpenGL 4.x versions
		for (int minor = 5; minor >= 0; --minor)
			if (initialize_context(Version{4, (uint32_t)minor}))
				break;

		// try OpenGL 3.x versions
		if (!this->handle)
			for (int minor = 3; minor >= 0; --minor)
				if (initialize_context(Version{3, (uint32_t)minor}))
					break;

		// try OpenGL 2.x versions
		if (!this->handle)
			for (int minor = 1; minor >= 0; --minor)
				if (initialize_context(Version{2, (uint32_t)minor}))
					break;

		if (!this->handle)
			throw dpl::GeneralException(this, __LINE__, "Fail to create OpenGL context: OpenGL is not supported on your machine.");

		ilInit();
		iluInit();
		if (ILenum error = ilGetError())
			throw dpl::GeneralException(this, std::string("Fail to initialize DevIL image loader. ") + iluErrorString(error));
	}

	CLASS_DTOR		Context::~Context()
	{
		if (handle())
		{
			OnRelease(handle());
			handle	= nullptr;
			version->major = 0;
			version->minor = 0;
		}
	}

	std::string		Context::get_info() const
	{
		std::string	info = "----------------------------------------------------------------\n";
		info += "Graphics Successfully Initialized\n";
		info += "OpenGL Info\n";
		info += "    Version: "; info += (const char*)glGetString(GL_VERSION);					info += "\n";
		info += "     Vendor: "; info += (const char*)glGetString(GL_VENDOR);					info += "\n";
		info += "   Renderer: "; info += (const char*)glGetString(GL_RENDERER);					info += "\n";
		info += "    Shading: "; info += (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION); info += "\n";
		info += "Selected OpenGL context version: "; info += std::to_string(version().major) + "." + std::to_string(version().minor) + "\n";
		return		info += "----------------------------------------------------------------\n";
	}


	std::string		GLException::opengl_message(	const GLenum	ERROR_CODE)
	{
		const std::string ERROR_CODE_STR	= " OpenGL error[" + std::to_string(ERROR_CODE) + "]: ";
		const std::string ERROR_MESSAGE		= reinterpret_cast<const char*>(glewGetErrorString(ERROR_CODE));

		switch(ERROR_CODE)
		{
		case GL_INVALID_ENUM:					return ERROR_CODE_STR + " GL_INVALID_ENUM -> " + ERROR_MESSAGE;
		case GL_INVALID_VALUE:					return ERROR_CODE_STR + " GL_INVALID_VALUE -> " + ERROR_MESSAGE;
		case GL_INVALID_OPERATION:				return ERROR_CODE_STR + " GL_INVALID_OPERATION -> " + ERROR_MESSAGE;
		case GL_STACK_OVERFLOW:					return ERROR_CODE_STR + " GL_STACK_OVERFLOW -> " + ERROR_MESSAGE;
		case GL_STACK_UNDERFLOW:				return ERROR_CODE_STR + " GL_STACK_UNDERFLOW -> " + ERROR_MESSAGE;
		case GL_OUT_OF_MEMORY:					return ERROR_CODE_STR + " GL_OUT_OF_MEMORY -> " + ERROR_MESSAGE;
		case GL_INVALID_FRAMEBUFFER_OPERATION:	return ERROR_CODE_STR + " GL_INVALID_FRAMEBUFFER_OPERATION -> " + ERROR_MESSAGE;
		case GL_CONTEXT_LOST:					return ERROR_CODE_STR + " GL_CONTEXT_LOST -> " + ERROR_MESSAGE;
		case GL_TABLE_TOO_LARGE:				return ERROR_CODE_STR + " GL_TABLE_TOO_LARGE -> " + ERROR_MESSAGE;
		default:								return ERROR_CODE_STR + " unknown";
		}
	}


	bool		isDepth(			const GLenum		FORMAT)
	{
		if (FORMAT == GL_DEPTH_COMPONENT || FORMAT == GL_DEPTH_COMPONENT16
		|| FORMAT == GL_DEPTH_COMPONENT24 || FORMAT == GL_DEPTH_COMPONENT32
		|| FORMAT == GL_DEPTH_COMPONENT32F)
			return true;

		return false;
	}

	bool		isStencil(			const GLenum		FORMAT)
	{
		if (FORMAT == GL_DEPTH_STENCIL || FORMAT == GL_DEPTH24_STENCIL8 || FORMAT == GL_DEPTH32F_STENCIL8)
			return true;

		return false;
	}

	bool		isUNormColor(		const GLenum		FORMAT)
	{
		switch(FORMAT)
		{
		// Base Formats
		case GL_RED:
		case GL_RG:
		case GL_RGB:
		case GL_RGBA:
			return true;

		// Sized Internal Formats
		case GL_R8:
		case GL_R16:
		case GL_RG8:
		case GL_RG16:
		case GL_RGB4:
		case GL_RGB5:
		case GL_RGB8:
		case GL_RGB10:
		case GL_RGB12:
		case GL_RGBA8:
		case GL_RGBA12:
		case GL_RGBA16:
			return true;

		default:
			return false;
		}
	}

	bool		isSNormColor(		const GLenum		FORMAT)
	{
		switch(FORMAT)
		{
		// Sized Internal Formats
		case GL_R8_SNORM:
		case GL_R16_SNORM:
		case GL_RG8_SNORM:
		case GL_RG16_SNORM:
		case GL_RGB8_SNORM:
		case GL_RGB16_SNORM:
		case GL_RGBA8_SNORM:
		case GL_SRGB8:
		case GL_SRGB8_ALPHA8:
			return true;

		default:
			return false;
		}
	}

	bool		isFloatColor(		const GLenum		FORMAT)
	{
		switch(FORMAT)
		{
		// Sized Internal Formats
		case GL_R16F:
		case GL_RG16F:
		case GL_RGB16F:
		case GL_RGBA16F:
		case GL_R32F:
		case GL_RG32F:
		case GL_RGB32F:
		case GL_RGBA32F:
			return true;

		default:
			return false;
		}
	}

	bool		isSignedIntColor(	const GLenum		FORMAT)
	{
		switch(FORMAT)
		{
		case GL_R8I:
		case GL_R16I:
		case GL_R32I:
		case GL_RG8I:
		case GL_RG16I:
		case GL_RG32I:
		case GL_RGB8I:
		case GL_RGB16I:
		case GL_RGB32I:
		case GL_RGBA8I:
		case GL_RGBA16I:
		case GL_RGBA32I:
			return true;

		default:
			return false;
		}
	}

	bool		isUnsignedIntColor(	const GLenum		FORMAT)
	{
		switch(FORMAT)
		{
		// Sized Internal Formats
		case GL_R8UI:
		case GL_R16UI:
		case GL_R32UI:
		case GL_RG8UI:
		case GL_RG16UI:
		case GL_RG32UI:
		case GL_RGB8UI:
		case GL_RGB16UI:
		case GL_RGB32UI:
		case GL_RGBA8UI:
		case GL_RGBA16UI:
		case GL_RGBA32UI:
			return true;

		default:
			return false;
		}
	}
}