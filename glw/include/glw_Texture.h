#pragma once


#include "glw_Utilities.h"


namespace glw
{
	/**
		OpenGL texture wrapper.
	*/
	class Texture
	{
	public: // subtypes
		enum	Type
		{
			eTEXTURE_UNKNOWN				= GL_NONE,							/*  Unknown texture type. */

			eTEXTURE_1D						= GL_TEXTURE_1D,					/*	Images in this texture all are 1-dimensional. 
																					They have width, but no height or depth. */
			eTEXTURE_2D						= GL_TEXTURE_2D,					/*	Images in this texture all are 2-dimensional. 
																					They have width and height, but no depth. */
			eTEXTURE_3D						= GL_TEXTURE_3D,					/*	Images in this texture all are 3-dimensional. 
																					They have width, height, and depth. */
			eTEXTURE_RECTANGLE				= GL_TEXTURE_RECTANGLE,				/*	The image in this texture (only one image. 
																					No mipmapping) is 2-dimensional. 
																					Texture coordinates used for these textures 
																					are not normalized. */
			eTEXTURE_BUFFER					= GL_TEXTURE_BUFFER,				/*	The image in this texture (only one image. 
																					No mipmapping) is 1-dimensional. 
																					The storage for this data comes from a Buffer Object. */
			eTEXTURE_CUBE_MAP				= GL_TEXTURE_CUBE_MAP,				/*	There are exactly 6 distinct sets of 2D images, 
																					all of the same size. They act as 6 faces of a cube. */
			eTEXTURE_1D_ARRAY				= GL_TEXTURE_1D_ARRAY,				/*	Images in this texture all are 1-dimensional. 
																					However, it contains multiple sets of 1-dimensional 
																					images, all within one texture. 
																					The array length is part of the texture's size. */
			eTEXTURE_2D_ARRAY				= GL_TEXTURE_2D_ARRAY,				/*	Images in this texture all are 2-dimensional. 
																					However, it contains multiple sets of 2-dimensional 
																					images, all within one texture. 
																					The array length is part of the texture's size. */
			eTEXTURE_CUBE_MAP_ARRAY			= GL_TEXTURE_CUBE_MAP_ARRAY,		/*	Images in this texture are all cube maps. 
																					It contains multiple sets of cube maps, 
																					all within one texture. 
																					The array length * 6 (number of cube faces) 
																					is part of the texture size. */
			eTEXTURE_2D_MULTISAMPLE			= GL_TEXTURE_2D_MULTISAMPLE,		/*	The image in this texture 
																					(only one image. No mipmapping) is 2-dimensional. 
																					Each pixel in these images contains multiple samples 
																					instead of just one value. */
			eTEXTURE_2D_MULTISAMPLE_ARRAY	= GL_TEXTURE_2D_MULTISAMPLE_ARRAY,	/*	Combines 2D array and 2D multisample types. 
																					No mipmapping. */
		};

		using	Unit = uint32_t;

	public: // data
		dpl::ReadOnly<GLuint,	Texture> GL_ID;
		dpl::ReadOnly<Type,		Texture> type;
		dpl::ReadOnly<GLenum,	Texture> internalFormat;

	protected: // lifecycle
		CLASS_CTOR				Texture(				const Type				TYPE,
														const GLenum			INTERNAL_FORMAT);

		CLASS_CTOR				Texture(				const Texture&			OTHER) = delete;

		CLASS_CTOR				Texture(				Texture&&				other) noexcept;

		CLASS_DTOR virtual		~Texture();

		Texture&				operator=(				const Texture&			OTHER) = delete;

		inline Texture&			operator=(				Texture&&				other) noexcept
		{
			return operator=(dpl::Swap<Texture>(other));
		}

		Texture&				operator=(				dpl::Swap<Texture>		other) noexcept;

	public: // functions
		inline void				bind_for_reading(		const Unit				UNIT) const
		{
			bind(UNIT);
			on_bind_for_reading();
		}

		static void				bind_default(			const Unit				UNIT);

		static void				invalidate_bindings();

	protected: // functions
		inline void				swap_IDs(				Texture&				other)
		{
			std::swap(*GL_ID, *other.GL_ID);
		}

		void					regenerate(				const Type				NEW_TYPE,
														const GLenum			NEW_INTERNAL_FORMAT);

	private: // functions
		void					generate_glObj();

		void					release_glObj();

		void					bind(					const Unit				UNIT) const;

	private: // interface
		virtual void			on_bind_for_reading() const{}
	};
}