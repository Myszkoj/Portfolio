#include "..//include/glw_Texture.h"

#pragma warning( disable : 26451)
#pragma warning( disable : 26812)

namespace glw
{
	const uint32_t	NUM_TEXTURE_UNITS = 80; // minimum number of texture units

	//uint32_t		textureUnits[NUM_TEXTURE_UNITS]	= { 0 }; //<-- Causes bugs

//=====> Texture protected: // lifecycle
	CLASS_CTOR	Texture::Texture(				const Type				TYPE,
												const GLenum			INTERNAL_FORMAT)
		: GL_ID(0u)
		, type(TYPE)
		, internalFormat(INTERNAL_FORMAT)
	{
		generate_glObj();
	}

	CLASS_CTOR	Texture::Texture(				Texture&&				other) noexcept
		: GL_ID(other.GL_ID)
		, type(other.type)
		, internalFormat(other.internalFormat)	
	{
		other.GL_ID	= 0u;
		other.type	= Type::eTEXTURE_UNKNOWN;
	}

	CLASS_DTOR	Texture::~Texture()
	{
		release_glObj();
	}

	Texture&	Texture::operator=(				dpl::Swap<Texture>		other) noexcept
	{
		GL_ID.swap(other->GL_ID);
		type.swap(other->type);
		internalFormat.swap(other->internalFormat);
		return *this;
	}

//=====> Texture public: // functions
	void		Texture::invalidate_bindings()
	{
		//for(uint32_t unit = 0; unit < NUM_TEXTURE_UNITS; ++unit)
		//{
		//	textureUnits[unit] = 0;
		//}
	}

	void		Texture::bind_default(			const Unit				UNIT)
	{
		//if (textureUnits[UNIT] != 0)
		//{
			//textureUnits[UNIT] = 0;

			glActiveTexture(GL_TEXTURE0 + UNIT);
			glBindTexture(GL_TEXTURE_2D, 0);
			validate_opengl(__FILE__, __LINE__, "Fail to bind default.");
		//}
	}

//=====> Texture protected: // functions
	void		Texture::regenerate(			const Type				NEW_TYPE,
												const GLenum			NEW_INTERNAL_FORMAT)
	{
		type			= NEW_TYPE;
		internalFormat	= NEW_INTERNAL_FORMAT;
		release_glObj();
		generate_glObj();
	}

//=====> Texture private: // functions
	void		Texture::generate_glObj()
	{
		glGenTextures(1, &*GL_ID);
		validate_opengl(__FILE__, __LINE__, "Fail to generate.");
	}

	void		Texture::release_glObj()
	{
		if (GL_ID != 0u)
		{
			glDeleteTextures(1, &GL_ID());
			GL_ID = 0u;
			validate_opengl(__FILE__, __LINE__, "Fail to release.");
		}
	}

	void		Texture::bind(					const Unit				UNIT) const
	{
		//if (textureUnits[UNIT] != ID)
		//{
			//textureUnits[UNIT] = ID;

			glActiveTexture(GL_TEXTURE0 + UNIT);
			glBindTexture(type(), GL_ID);
			validate_opengl(__FILE__, __LINE__, "Fail to bind.");
		//}
	}
}