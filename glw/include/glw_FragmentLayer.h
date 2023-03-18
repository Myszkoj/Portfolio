#pragma once


#include "glw_RenderableTexture.h"


namespace glw
{
	class FrameBuffer;


	/*
		Variant of the attachable Framebuffer layer. 
	*/
	class FragmentLayer : public dpl::Variant<FrameBuffer, FragmentLayer>
						, public RenderableTexture
	{
	public: // relations
		friend FrameBuffer;

	public: // subtypes
		enum Attachment
		{		
			COLOR_BUFFER_0			= GL_COLOR_ATTACHMENT0,
			COLOR_BUFFER_1			= GL_COLOR_ATTACHMENT1,
			COLOR_BUFFER_2			= GL_COLOR_ATTACHMENT2,
			COLOR_BUFFER_3			= GL_COLOR_ATTACHMENT3,
			COLOR_BUFFER_4			= GL_COLOR_ATTACHMENT4,
			COLOR_BUFFER_5			= GL_COLOR_ATTACHMENT5,
			COLOR_BUFFER_6			= GL_COLOR_ATTACHMENT6,
			COLOR_BUFFER_7			= GL_COLOR_ATTACHMENT7,
				
			COLOR_BUFFER_8			= GL_COLOR_ATTACHMENT8,
			COLOR_BUFFER_9			= GL_COLOR_ATTACHMENT9,
			COLOR_BUFFER_10			= GL_COLOR_ATTACHMENT10,
			COLOR_BUFFER_11			= GL_COLOR_ATTACHMENT11,
			COLOR_BUFFER_12			= GL_COLOR_ATTACHMENT12,
			COLOR_BUFFER_13			= GL_COLOR_ATTACHMENT13,
			COLOR_BUFFER_14			= GL_COLOR_ATTACHMENT14,
			COLOR_BUFFER_15			= GL_COLOR_ATTACHMENT15,

			DEPTH_BUFFER			= GL_DEPTH_ATTACHMENT,
			STENCIL_BUFFER			= GL_STENCIL_ATTACHMENT,
			DEPTH_STENCIL_BUFFER	= GL_DEPTH_STENCIL_ATTACHMENT
		};

		using Binding = Variant::Binding;

	public: // data
		dpl::ReadOnly<Attachment, FragmentLayer> attachment;

	protected: // lifecycle
		CLASS_CTOR		FragmentLayer(		const Binding&		BINDING,
											const Attachment	ATTACHMENT,
											const GLenum		INTERNAL_FORMAT)
			: Variant(BINDING)
			, RenderableTexture(Type::eTEXTURE_2D, Resolution(MIN_SIZE, MIN_SIZE), 1, INTERNAL_FORMAT, Filter::eNEAREST, WrapMode::eCLAMP_TO_EDGE)
			, attachment(ATTACHMENT)
		{
			if(this->is_color_buffer())
			{
				GLint maxValue = 0;
				glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxValue);
				auto drawID = attachment() - COLOR_BUFFER_0;
				if(drawID > maxValue)
					throw dpl::GeneralException(this, __LINE__, "Unsupported attachment index: " + std::to_string(drawID));
			}
		}

	public: // functions
		inline bool		is_color_buffer() const
		{
			return attachment() >= COLOR_BUFFER_0 && attachment() <= COLOR_BUFFER_15;
		}

		inline bool		is_depth_attachment() const
		{
			return attachment() == GL_DEPTH_ATTACHMENT || attachment() == GL_DEPTH_STENCIL_ATTACHMENT;
		}

		inline bool		is_stencil_attachment() const
		{
			return attachment() == GL_STENCIL_ATTACHMENT || attachment() == GL_DEPTH_STENCIL_ATTACHMENT;
		}

	private: // functions
		inline void		swap_textures(		FragmentLayer&		other)
		{
			if(*this == other) swap_IDs(other);
		}

		inline void		update(				const Resolution&	RESOLUTION) const
		{
			if(resolution().x == 0 || resolution().y == 0)
			{
				// OpenGL crashes if width or height of the texture is 0.
				const_cast<FragmentLayer&>(*this).rebuild(Resolution(2, 2));
			}
			else
			{
				const_cast<FragmentLayer&>(*this).rebuild(RESOLUTION);
			}
		}

	private: // interface
		virtual void	clear() = 0;
	};
}