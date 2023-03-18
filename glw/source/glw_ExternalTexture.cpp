#include "..//include/glw_ExternalTexture.h"

#pragma warning( disable : 26451)
#pragma warning( disable : 26812)

namespace glw
{
	using IndexSizeT = uint64_t; // import/export purpose


	CLASS_CTOR			ExternalTexture::Source::Source(				const std::string&		FILE_NAME)
		: fileName(FILE_NAME)
		, resolution(0, 0)
		, format(IL_RGBA)
	{
		ilGenImages(1, &(*id));
		if (id() == 0)
			throw dpl::GeneralException(this, __LINE__, "Fail to generate Ilu image: " + FILE_NAME + " DevIL error: " + iluErrorString(ilGetError()));
		
		ilBindImage(id);

		if (!ilLoadImage(FILE_NAME.c_str()))
		{
			const ILenum ERROR_CODE = ilGetError();
			throw dpl::GeneralException(this, __LINE__, "Fail to load Ilu image: " + FILE_NAME + " DevIL error: " + iluErrorString(ERROR_CODE));
		}

		resolution->x	= ilGetInteger(IL_IMAGE_WIDTH);
		resolution->y	= ilGetInteger(IL_IMAGE_HEIGHT);
		format			= ilGetInteger(IL_IMAGE_FORMAT);

		if(format() == IL_COLOR_INDEX)
			throw dpl::GeneralException(this, __LINE__, "COLOR_INDEX textures are not supported. File: " + FILE_NAME);
	}

	CLASS_DTOR			ExternalTexture::Source::~Source()
	{
		if(id() != 0)
		{
			ilDeleteImages(1, &(*id));
			id = 0;
		}
	}

	const ILubyte*		ExternalTexture::Source::get_pixels() const
	{
		const ILubyte* BYTES = ilGetData();
		if(!BYTES)
			throw dpl::GeneralException(this, __LINE__, "Fail to extract data from Ilu. File: " + fileName());

		return BYTES;
	}

	CLASS_CTOR			ExternalTexture::SourceArray::SourceArray(		const std::string*		FILES,
																		const uint32_t			NUM_FILES)
		: resolution(0, 0)
		, format(IL_RGBA)
	{
		if(!FILES && NUM_FILES > 0)
			throw dpl::GeneralException("Source files are missing.");

		sources->reserve(NUM_FILES);
		for(uint32_t index = 0; index < NUM_FILES; ++index)
		{
			auto& newSource = sources->emplace_back(FILES[index]);

			if(index == 0)
			{
				resolution	= newSource.resolution();
				format		= newSource.format();
			}
			else
			{
				if(newSource.format() != sources()[0].format())
					throw dpl::GeneralException(this, __LINE__, "All source files must have the same format.");

				if(newSource.resolution() != sources()[0].resolution())
					throw dpl::GeneralException(this, __LINE__, "All source files must have the same resolution.");
			}
		}
	}

	CLASS_CTOR			ExternalTexture::CubeMapSource::CubeMapSource(	const std::string&		POSITIVE_X_FILE_NAME,
																		const std::string&		NEGATIVE_X_FILE_NAME,
																		const std::string&		POSITIVE_Y_FILE_NAME,
																		const std::string&		NEGATIVE_Y_FILE_NAME,
																		const std::string&		POSITIVE_Z_FILE_NAME,
																		const std::string&		NEGATIVE_Z_FILE_NAME)
		: POSITIVE_X(POSITIVE_X_FILE_NAME)
		, NEGATIVE_X(NEGATIVE_X_FILE_NAME)
		, POSITIVE_Y(POSITIVE_Y_FILE_NAME)
		, NEGATIVE_Y(NEGATIVE_Y_FILE_NAME)
		, POSITIVE_Z(POSITIVE_Z_FILE_NAME)
		, NEGATIVE_Z(NEGATIVE_Z_FILE_NAME)
		, resolution(0, 0)
		, format(IL_RGBA)
	{
		auto assert_format = [&](	const Source& FIRST,
									const Source& SECOND)
		{
			if(FIRST.format() != SECOND.format())
				throw dpl::GeneralException(this, __LINE__, "All source files must have the same format.");

			if(FIRST.resolution() != SECOND.resolution())
				throw dpl::GeneralException(this, __LINE__, "All source files must have the same resolution.");
		};

		assert_format(POSITIVE_X, NEGATIVE_X);
		assert_format(POSITIVE_X, POSITIVE_Y);
		assert_format(POSITIVE_X, NEGATIVE_Y);
		assert_format(POSITIVE_X, POSITIVE_Z);
		assert_format(POSITIVE_X, NEGATIVE_Z);

		resolution	= POSITIVE_X().resolution();
		format		= POSITIVE_X().format();
	}

//=====> ExternalTexture public: // lifecycle
	CLASS_CTOR			ExternalTexture::ExternalTexture(				const dpl::Ownership&		OWNERSHIP,
																		const Binding&				BINDING)
		: RenderableTexture(Type::eTEXTURE_2D, Resolution(MIN_SIZE, MIN_SIZE), 1, GL_RGBA)
		, Resource(OWNERSHIP, BINDING)
		, sourceType(SourceType::UNDEFINED)
	{
		
	}

	CLASS_CTOR			ExternalTexture::ExternalTexture(				const dpl::Ownership&		OWNERSHIP,
																		const Binding&				BINDING,
																		const Source&				SOURCE,
																		const Filter				FILTER, 
																		const WrapMode				WRAP_MODE)
		: RenderableTexture(Type::eTEXTURE_2D, SOURCE.resolution(), 1, SOURCE.format(), FILTER, WRAP_MODE)
		, Resource(OWNERSHIP, BINDING)
		, sourceType(SourceType::SINGLE_SOURCE)
	{
		initialize(SOURCE);
	}

	CLASS_CTOR			ExternalTexture::ExternalTexture(				const dpl::Ownership&		OWNERSHIP,
																		const Binding&				BINDING,
																		const SourceArray&			SOURCE_ARRAY,
																		const Filter				FILTER, 
																		const WrapMode				WRAP_MODE)
		: RenderableTexture(Type::eTEXTURE_2D_ARRAY, SOURCE_ARRAY.resolution(), static_cast<uint32_t>(SOURCE_ARRAY.sources().size()), SOURCE_ARRAY.format(), FILTER, WRAP_MODE)
		, Resource(OWNERSHIP, BINDING)
		, sourceType(SourceType::SOURCE_ARRAY)
	{
		initialize(SOURCE_ARRAY);
	}

	CLASS_CTOR			ExternalTexture::ExternalTexture(				const dpl::Ownership&		OWNERSHIP,
																		const Binding&				BINDING,
																		const CubeMapSource&		CUBE_MAP,
																		const Filter				FILTER, 
																		const WrapMode				WRAP_MODE)
		: RenderableTexture(Type::eTEXTURE_CUBE_MAP, CUBE_MAP.resolution(), 6, CUBE_MAP.format(), FILTER, WRAP_MODE)
		, Resource(OWNERSHIP, BINDING)
		, sourceType(SourceType::CUBEMAP_SOURCE)
	{
		initialize(CUBE_MAP);
	}

	CLASS_CTOR			ExternalTexture::ExternalTexture(				const dpl::Ownership&		OWNERSHIP, 
																		ExternalTexture&&			other) noexcept
		: RenderableTexture(std::move(other))
		, Resource(OWNERSHIP, std::move(other))
		, sourceType(other.sourceType)
		, paths(std::move(other.paths))
	{
		other.sourceType = SourceType::UNDEFINED;
	}

	ExternalTexture&	ExternalTexture::operator=(						dpl::Swap<ExternalTexture>	other)
	{
		Texture::operator=(dpl::Swap<Texture>(*other));
		Resource::operator=(dpl::Swap(*other));
		sourceType.swap(other->sourceType);
		paths.swap(other->paths);
		return *this;
	}

//=====> Texture private: // Resource implementation
	void				ExternalTexture::import_from(					std::istream&				binary)
	{
		import_label(binary);

		// Samplable data
		dpl::import_t(binary, type);
		dpl::import_t(binary, internalFormat);

		// Texture data
		dpl::import_t(binary, resolution);
		dpl::import_t(binary, depth);
		dpl::import_t(binary, filter);
		dpl::import_t(binary, wrapMode);

		// Source data
		dpl::import_t(binary, sourceType);
		const auto NUM_SOURCE_FILES = dpl::import_t<IndexSizeT>(binary);
		paths->resize(NUM_SOURCE_FILES);
		for(IndexSizeT index = 0; index < NUM_SOURCE_FILES; ++index)
		{
			dpl::import_dynamic_container(binary, paths->at(index));
		}
		
		if(sourceType() != SourceType::UNDEFINED)
		{
			if(paths().empty())
			{
				throw dpl::GeneralException(this, __LINE__, "Texture file path(s) are missing.");
			}

			rebuild(resolution, true);

			switch(sourceType())
			{
			case SourceType::SINGLE_SOURCE:		
				initialize(Source(paths().front()));	
				break;

			case SourceType::SOURCE_ARRAY:		
				initialize(SourceArray(paths()));		
				break;

			case SourceType::CUBEMAP_SOURCE:	
				if(paths().size() == 6)
				{
					initialize(CubeMapSource(paths()));		
					break;
				}
					
			default: throw dpl::GeneralException(this, __LINE__, "Fail to import texture.");
			}
		}
	}

	void				ExternalTexture::export_to(						std::ostream&				binary) const
	{
		export_label(binary);

		// Samplable data
		dpl::export_t(binary, type);
		dpl::export_t(binary, internalFormat);

		// Texture data
		dpl::export_t(binary, resolution);
		dpl::export_t(binary, depth);
		dpl::export_t(binary, filter);
		dpl::export_t(binary, wrapMode);

		// Source data
		dpl::export_t(binary, sourceType);
		const auto NUM_SOURCE_FILES = (IndexSizeT)paths().size();
		dpl::export_t(binary, NUM_SOURCE_FILES);
		for(IndexSizeT index = 0; index < NUM_SOURCE_FILES; ++index)
		{
			dpl::export_container(binary, paths()[index]);
		}
	}

//=====> ExternalTexture private: // functions
	void				ExternalTexture::initialize(					const Source&				SOURCE)
	{
		glTexSubImage2D(type(), 0, 0, 0, resolution().x, resolution().y, SOURCE.format(), GL_UNSIGNED_BYTE, SOURCE.get_pixels());
		validate_opengl(__FILE__, __LINE__, "Fail to create texture from given source.");
	}

	void				ExternalTexture::initialize(					const SourceArray&			SOURCE_ARRAY)
	{
		// This is already done in the default constructor.
		//glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat(), SOURCE_ARRAY.resolution().x, SOURCE_ARRAY.resolution().y, depth(), 0, SOURCE_ARRAY.format(), GL_UNSIGNED_BYTE, NULL);

		for(uint32_t index = 0; index < SOURCE_ARRAY.sources().size(); ++index)
		{
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, resolution().x, resolution().y, 1, SOURCE_ARRAY.format(), GL_UNSIGNED_BYTE, SOURCE_ARRAY.sources()[index].get_pixels());
		}

		validate_opengl(__FILE__, __LINE__, "Fail to create texture array.");
	}

	void				ExternalTexture::initialize(					const CubeMapSource&		CUBE_MAP)
	{
		auto load_side = [&](	const GLenum	SIDE,
								const Source&	SOURCE)
		{
			glTexImage2D(SIDE, 0, internalFormat(), resolution().x, resolution().y, 0, CUBE_MAP.format(), GL_UNSIGNED_BYTE, SOURCE.get_pixels());
		};

		load_side(GL_TEXTURE_CUBE_MAP_POSITIVE_X, CUBE_MAP.POSITIVE_X);
		load_side(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, CUBE_MAP.NEGATIVE_X);
		load_side(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, CUBE_MAP.POSITIVE_Y);
		load_side(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, CUBE_MAP.NEGATIVE_Y);
		load_side(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, CUBE_MAP.POSITIVE_Z);
		load_side(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, CUBE_MAP.NEGATIVE_Z);

		validate_opengl(__FILE__, __LINE__, "Fail to create cubemap texture.");
	}
}