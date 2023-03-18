#pragma once


#include "glw_Texture.h"
#include "glw_Buffer.h"


namespace glw
{
	template<typename glslT>
	class TextureBuffer	: public Texture
						, public Buffer_of<typename glslT::DataT>
	{
	private: // subdata
		using MyBufferBase	= Buffer_of<typename glslT::DataT>;
		using MyFormat		= typename glslT::Format;

	public: // lifecycle
		CLASS_CTOR			TextureBuffer(	const Buffer::Usage	USAGE)
			: Texture(Texture::eTEXTURE_BUFFER, MyFormat::GL)
			, MyBufferBase(Buffer::TEXTURE_BUFFER, USAGE) //<-- Note that Texture::TEXTURE_BUFFER != IBuffer::TEXTURE_BUFFER!
		{		
			validate_opengl(__FILE__, __LINE__, "Construction failure.");
		}

		CLASS_CTOR			TextureBuffer(	const TextureBuffer&	OTHER) = delete;

		CLASS_CTOR			TextureBuffer(	TextureBuffer&&			other) noexcept = default;

		TextureBuffer&		operator=(		const TextureBuffer&	OTHER) = delete;

		TextureBuffer&		operator=(		TextureBuffer&&			other) noexcept = default;

	private: // implementation
		virtual void		on_bind_for_reading() const final override
		{
			MyBufferBase::bind();
		}

		virtual void		on_updated() const final override
		{
			// NOTE: Shader sees zeros if buffer is empty at the time it is associated with the texture.
			// https://github.com/cginternals/globjects/issues/160
			glTexBufferARB(Texture::type(), MyFormat::GL, Buffer::ID);
			validate_opengl(__FILE__, __LINE__, "This storage format is not supported.");
		}
	};
}