#include "..//include/glw_RenderableTexture.h"

#pragma warning( disable : 26451)

namespace glw
{
	const RenderableTexture::Resolution RenderableTexture::MIN_RESOLUTION(RenderableTexture::MIN_SIZE, RenderableTexture::MIN_SIZE);


//=====> RenderableTexture public: // lifecycle
	CLASS_CTOR			RenderableTexture::RenderableTexture(	const Type						TYPE, 
																const Resolution&				RESOLUTION,
																const uint32_t					DEPTH,
																const GLenum					INTERNAL_FORMAT,
																const Filter					FILTER, 
																const WrapMode					WRAP_MODE)
		: Texture(TYPE, INTERNAL_FORMAT)
		, resolution(0, 0) //<-- Must be different than RESOLUTION, otherwise rebuild won't work.
		, depth(DEPTH)
		, filter(FILTER)
		, wrapMode(WRAP_MODE)
	{
		rebuild(RESOLUTION);
	}

	RenderableTexture&	RenderableTexture::operator=(			dpl::Swap<RenderableTexture>	other) noexcept
	{
		Texture::operator=(dpl::Swap<Texture>(*other));
		resolution.swap(other->resolution);
		depth.swap(other->depth);
		filter.swap(other->filter);
		wrapMode.swap(other->wrapMode);
		return *this;
	}

//=====> RenderableTexture public: // functions
	bool				RenderableTexture::operator==(			const RenderableTexture&		OTHER) const
	{
		if(this->type			!= OTHER.type)				return false;
		if(this->internalFormat != OTHER.internalFormat)	return false;
		if(this->resolution		!= OTHER.resolution)		return false;
		if(this->depth			!= OTHER.depth)				return false;
		if(this->filter			!= OTHER.filter)			return false;
		if(this->wrapMode		!= OTHER.wrapMode)			return false;
		return true;
	}

//=====> RenderableTexture protected: // functions
	void				RenderableTexture::rebuild(				Resolution						newResolution,
																const bool						bFORCE_REBUILD)
	{
		newResolution.x = glm::clamp(newResolution.x, MIN_SIZE, MAX_SIZE);
		newResolution.y = glm::clamp(newResolution.y, MIN_SIZE, MAX_SIZE);

		if(!bFORCE_REBUILD && (resolution == newResolution))
			return; // No need to rebuild.

		resolution = newResolution;

		regenerate(type(), internalFormat());
		bind_for_reading(0);
		set_parameters();
		set_storage();
		validate_opengl(__FILE__, __LINE__, "Fail to rebuild.");
	}

//=====> RenderableTexture private: // functions
	void				RenderableTexture::set_parameters()
	{
		glTexParameteri(type(), GL_TEXTURE_MIN_FILTER,	filter());
		glTexParameteri(type(), GL_TEXTURE_MAG_FILTER,	filter());
		glTexParameteri(type(), GL_TEXTURE_WRAP_S,		wrapMode());
		glTexParameteri(type(), GL_TEXTURE_WRAP_T,		wrapMode());
		glTexParameteri(type(), GL_TEXTURE_WRAP_R,		wrapMode());
		validate_opengl(__FILE__, __LINE__, "Fail to setup texture parameters.");
	}

	void				RenderableTexture::set_storage()
	{
		/*
		*	For opengl 4.2 glTexStorage2D can be used to ignore format and data type.
		*/

		GLenum format	= GL_RGBA; 
		GLenum dataType = GL_UNSIGNED_BYTE;

		if(isUnsignedIntColor(internalFormat()))
		{
			format		= GL_RED_INTEGER;
			dataType	= GL_UNSIGNED_INT;
		}
		else if(isSignedIntColor(internalFormat()))
		{
			format		= GL_RED_INTEGER;
			dataType	= GL_INT;
		}
		else if(isFloatColor(internalFormat()))
		{
			format		= GL_RED;
			dataType	= GL_FLOAT;
		}
		else
		{
			switch(internalFormat())
			{
			case	GL_DEPTH_COMPONENT16:
				format		= GL_DEPTH_COMPONENT;
				dataType	= GL_UNSIGNED_SHORT;

			case	GL_DEPTH_COMPONENT24:
				format		= GL_DEPTH_COMPONENT;
				dataType	= GL_UNSIGNED_INT_24_8; // experimental
				break;

			case	GL_DEPTH_COMPONENT32:
				format		= GL_DEPTH_COMPONENT;
				dataType	= GL_UNSIGNED_INT;
				break;

			case	GL_DEPTH_COMPONENT32F:
				format		= GL_DEPTH_COMPONENT;
				dataType	= GL_FLOAT; // experimental, may cause errors, try GL_UNSIGNED_INT
				break;

			case	GL_DEPTH24_STENCIL8:
				format		= GL_DEPTH_STENCIL;
				dataType	= GL_UNSIGNED_INT_24_8;
				break;

			case	GL_DEPTH32F_STENCIL8:
				format		= GL_DEPTH_STENCIL;
				dataType	= GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
				break;

			default:
				break;
			}
		}
			
		if(type() == GL_TEXTURE_2D)
		{
			glTexImage2D(type(), 0, internalFormat(), resolution().x, resolution().y, 0, format, dataType, NULL);
		}
		else if(type() == GL_TEXTURE_3D || type() == GL_TEXTURE_2D_ARRAY)
		{
			glTexImage3D(type(), 0, internalFormat(), resolution().x, resolution().y, depth(), 0, format, dataType, NULL);
		}
		else if (type() == GL_TEXTURE_CUBE_MAP)
		{
			for (int n = 0; n < 6; ++n)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + n, 0, internalFormat(), resolution().x, resolution().y, 0, format, dataType, NULL);
			}
		}
	}
}