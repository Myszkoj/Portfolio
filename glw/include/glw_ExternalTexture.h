#pragma once


#include <dpl_ResourceControl.h>
#include "glw_RenderableTexture.h"


namespace glw
{
	class ExternalTexture	: public RenderableTexture
							, public dpl::Resource<ExternalTexture>
	{
	public: // subtypes		
		class	Source
		{
		public: // data
			dpl::ReadOnly<std::string,	Source> fileName;
			dpl::ReadOnly<Resolution,	Source>	resolution;
			dpl::ReadOnly<GLenum,		Source>	format;
			dpl::ReadOnly<ILuint,		Source> id;

		public: // functions
			CLASS_CTOR		Source(		const std::string&	FILE_NAME);

			CLASS_CTOR		Source(		const char*			FILE_NAME)
				: Source(std::string(FILE_NAME))
			{

			}

			CLASS_DTOR		~Source();

		public: // data
			const ILubyte*	get_pixels() const;
		};

		class	SourceArray
		{
		public: // data
			dpl::ReadOnly<Resolution,			SourceArray> resolution;
			dpl::ReadOnly<GLenum,				SourceArray> format;
			dpl::ReadOnly<std::vector<Source>,	SourceArray> sources;

		public: // lifecycle
			CLASS_CTOR			SourceArray(	const std::string*				FILES,
												const uint32_t					NUM_FILES);

			CLASS_CTOR			SourceArray(	const std::vector<std::string>	FILES)
				: SourceArray(FILES.data(), static_cast<uint32_t>(FILES.size()))
			{

			}

		public: // functions
			const std::string&	name() const
			{
				return sources().at(0).fileName();
			}
		};

		class	CubeMapSource
		{
		public: // data
			dpl::ReadOnly<Resolution,	CubeMapSource>	resolution;
			dpl::ReadOnly<GLenum,		CubeMapSource>	format;
			dpl::ReadOnly<Source,		CubeMapSource>	POSITIVE_X;
			dpl::ReadOnly<Source,		CubeMapSource>	NEGATIVE_X;
			dpl::ReadOnly<Source,		CubeMapSource>	POSITIVE_Y;
			dpl::ReadOnly<Source,		CubeMapSource>	NEGATIVE_Y;
			dpl::ReadOnly<Source,		CubeMapSource>	POSITIVE_Z;
			dpl::ReadOnly<Source,		CubeMapSource>	NEGATIVE_Z;

		public: // lifecycle
			CLASS_CTOR	CubeMapSource(	const std::string&	POSITIVE_X_FILE_NAME,
										const std::string&	NEGATIVE_X_FILE_NAME,
										const std::string&	POSITIVE_Y_FILE_NAME,
										const std::string&	NEGATIVE_Y_FILE_NAME,
										const std::string&	POSITIVE_Z_FILE_NAME,
										const std::string&	NEGATIVE_Z_FILE_NAME);

			CLASS_CTOR	CubeMapSource(	const std::vector<std::string>	FILES)
				: CubeMapSource(FILES[0], FILES[1], FILES[2], FILES[3], FILES[4], FILES[5])
			{

			}
		};

		enum	SourceType
		{
			UNDEFINED,
			SINGLE_SOURCE,
			SOURCE_ARRAY,
			CUBEMAP_SOURCE
		};

	public: // data
		dpl::ReadOnly<SourceType,				ExternalTexture> sourceType;
		dpl::ReadOnly<std::vector<std::string>,	ExternalTexture> paths;

	public: // lifecycle
		/**
			Constructs invalid texture(ready to be imported from the file).
		*/
		CLASS_CTOR			ExternalTexture(const dpl::Ownership&		OWNERSHIP,
											const Binding&				BINDING);

		/**
			Constructs 2D Texture from given file.
		*/
		CLASS_CTOR			ExternalTexture(const dpl::Ownership&		OWNERSHIP,
											const Binding&				BINDING,
											const Source&				SOURCE,
											const Filter				FILTER		= Filter::eLINEAR, 
											const WrapMode				WRAP_MODE	= WrapMode::eREPEAT);

		/**
			Constructs texture array from given files.
		*/
		CLASS_CTOR			ExternalTexture(const dpl::Ownership&		OWNERSHIP,
											const Binding&				BINDING,
											const SourceArray&			SOURCE_ARRAY,
											const Filter				FILTER		= Filter::eLINEAR, 
											const WrapMode				WRAP_MODE	= WrapMode::eREPEAT);

		/**
			Constructs Cubemap texture from given files.
		*/
		CLASS_CTOR			ExternalTexture(const dpl::Ownership&		OWNERSHIP,
											const Binding&				BINDING,
											const CubeMapSource&		CUBE_MAP,
											const Filter				FILTER		= Filter::eLINEAR, 
											const WrapMode				WRAP_MODE	= WrapMode::eREPEAT);

		CLASS_CTOR			ExternalTexture(const dpl::Ownership&		OWNERSHIP, 
											ExternalTexture&&			other) noexcept;

		ExternalTexture&	operator=(		dpl::Swap<ExternalTexture>	other);

	private: // lifecycle
		CLASS_CTOR			ExternalTexture(ExternalTexture&&			other) noexcept = delete;

		ExternalTexture&	operator=(		ExternalTexture&&			other) noexcept = delete;

	public: // import/export
		void				import_from(	std::istream&				binary);

		void				export_to(		std::ostream&				binary) const;

	private: // functions
		void				initialize(		const Source&				SOURCE);

		void				initialize(		const SourceArray&			SOURCE_ARRAY);

		void				initialize(		const CubeMapSource&		CUBE_MAP);
	};
}