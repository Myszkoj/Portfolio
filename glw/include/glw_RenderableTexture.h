#pragma once


#include "glw_Texture.h"


namespace glw
{
	/**
		OpenGL texture wrapper.

		// Filter and wrap mode.
		https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glTexParameter.xml
	*/
	class RenderableTexture	: public Texture
	{
	public: // subtypes
		enum	Filter
		{
			eNEAREST				= GL_NEAREST,
			eLINEAR					= GL_LINEAR,
			eNEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
			eNEAREST_MIPMAP_LINEAR	= GL_NEAREST_MIPMAP_LINEAR,
			eLINEAR_MIPMAP_LINEAR	= GL_LINEAR_MIPMAP_LINEAR,
			eLINEAR_MIPMAP_NEAREST	= GL_LINEAR_MIPMAP_NEAREST
		};

		enum	WrapMode
		{
			eCLAMP_TO_EDGE		= GL_CLAMP_TO_EDGE,
			eREPEAT				= GL_REPEAT,
			eMIRRORED_REPEAT	= GL_MIRRORED_REPEAT
		};

		using	Resolution	= glm::tvec2<uint32_t>;

	public: // constants
		static const uint32_t	MIN_SIZE = 2;	// Min width or height.
		static const uint32_t	MAX_SIZE = 8192;// Max width or height.
		static const Resolution MIN_RESOLUTION;

	public: // data
		dpl::ReadOnly<Resolution,	RenderableTexture> resolution; // Width and height of the texture.
		dpl::ReadOnly<uint32_t,		RenderableTexture> depth;
		dpl::ReadOnly<Filter,		RenderableTexture> filter; 
		dpl::ReadOnly<WrapMode,		RenderableTexture> wrapMode;

	public: // lifecycle
		CLASS_CTOR			RenderableTexture(		const Type						TYPE, 
													const Resolution&				RESOLUTION,
													const uint32_t					DEPTH,
													const GLenum					INTERNAL_FORMAT,
													const Filter					FILTER		= Filter::eLINEAR, 
													const WrapMode					WRAP_MODE	= WrapMode::eREPEAT);

		CLASS_CTOR			RenderableTexture(		RenderableTexture&&				other) noexcept = default;

		RenderableTexture&	operator=(				RenderableTexture&&				other) noexcept = default;

		RenderableTexture&	operator=(				dpl::Swap<RenderableTexture>	other) noexcept;

	public: // functions
		bool				operator==(				const RenderableTexture&		OTHER) const;

		inline bool			operator!=(				const RenderableTexture&		OTHER) const
		{
			return !(*this == OTHER);
		}

	protected: // functions
		void				rebuild(				Resolution						newResolution,
													const bool						bFORCE_REBUILD = false);

	private: // functions
		void				set_parameters();

		void				set_storage();
	};
}