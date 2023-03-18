#include "..//include/glw_SLStage.h"
#include <sstream>


namespace glw
{
	/*
	Stage::Type		get_shader_type(	const Stage::Name&		NAME)
	{
		std::string		typeStr = get_fileExtension(NAME);
		Stage::Type		type	= Stage::Type::eUNKNOWN_SHADER;

		if		(typeStr == ".vsh" || typeStr == "VSH")
		{	
			type = Stage::Type::eVERTEX_SHADER;
		}
		else if (typeStr == ".fsh" || typeStr == "FSH")
		{
			type = Stage::Type::eFRAGMENT_SHADER;
		}
		else if (typeStr == ".gsh" || typeStr == "GSH")
		{
			type = Stage::Type::eGEOMETRY_SHADER;
		}
		else
		{
			throw dpl::GeneralException(__FILE__, __LINE__, "Fail to read shader type: " + NAME);
		}

		return type;
	}
	*/

//=====> SLStage public: // lifecycle
	CLASS_CTOR		SLStage::SLStage(				const SLBlueprint&				BLUEPRINT)
		: type(BLUEPRINT.type())
		, ID(0)
	{
		generate_and_compile(BLUEPRINT.type(), BLUEPRINT.generate_source(), BLUEPRINT.name());
		BLUEPRINT.for_each_external_transfer([&](ExternalSLTransfer& transfer)
		{
			add_subject(transfer);
		});
	}

	CLASS_CTOR		SLStage::SLStage(				SLStage&&						other) noexcept
		: ExternalStreams(std::move(other))
		, type(other.type)
		, ID(other.ID)
	{
		other.ID = 0;
	}

	CLASS_DTOR		SLStage::~SLStage()
	{
		release_glObj();
	}

	SLStage&		SLStage::operator=(				dpl::Swap<SLStage>				other) noexcept
	{
		ExternalStreams::operator=(dpl::Swap<ExternalStreams>(*other));
		type.swap(other->type);
		ID.swap(other->ID);
		return *this;
	}

//=====> SLStage private: // functions
	std::string		SLStage::get_shader_code(		const SourceCode&				source) const
	{
		int infoLogLength	= 0;
		int maxLength		= 0;

		glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &maxLength);

		std::unique_ptr<char[]> buffer(new char[maxLength]);

		glGetShaderInfoLog(ID, maxLength, &infoLogLength, buffer.get());

		std::string			errorMessage(buffer.get(), infoLogLength);
		std::string			codeView;
		std::string			tmp;
		uint32_t			lineNumber = 0;
		std::istringstream	iss(source);

		if(errorMessage.size() > 0)
		{
			codeView += errorMessage + "[0]\n"; // <-- Line numbers in error messages include the empty line in between.
			++lineNumber;
		}

		while (std::getline(iss, tmp))
		{
			//if (tmp.size() > 0)
			//	if (tmp.size() >= 2)
			//		if (tmp[0] == '/' && tmp[1] == '/')
			//			continue;

			codeView += "[" + std::to_string(lineNumber) + "]:\t" + tmp + '\n';
			++lineNumber;
		}

		return codeView;
		//return errorMessage + "\n----------> Shader code:\n" + codeView + "\n<----------\n";
	}

	void			SLStage::compile(				const SourceCode&				SOURCE,
													const std::string&				LABEL)
	{
		if(ID() == 0)
			throw dpl::GeneralException(this, __LINE__, "Fail to compile. You must first generate shader.");

		const char* buffer	= SOURCE.c_str();
		GLint		size	= static_cast<GLint>(SOURCE.size());

		glShaderSource(ID, 1, &buffer, &size);
		glCompileShader(ID);
		validate_opengl(__FILE__, __LINE__, "Fail to compile shader.");
		GLint compiled = GL_FALSE;
		glGetShaderiv(ID, GL_COMPILE_STATUS, &compiled);

		if(compiled != GL_TRUE)
		{
			const std::string DEBUG_FILE_NAME = LABEL + "_debug.txt";

			std::ofstream logFile(DEBUG_FILE_NAME);
			logFile << get_shader_code(SOURCE);
			logFile.close();

			throw dpl::GeneralException(this, __LINE__, "Fail to compile shader, output file: " + DEBUG_FILE_NAME);
		}
	}

	void			SLStage::generate_and_compile(	const Type						TYPE,
													const std::string&				SOURCE_CODE,
													const std::string&				LABEL)
	{
		if(ID() != 0)
			throw dpl::GeneralException(this, __LINE__, "Shader cannot be generated twice.");

		ID = glCreateShader(type);

		validate_opengl(__FILE__, __LINE__, "Fail to generate. SOURCE_CODE: \n" + SOURCE_CODE);
		compile(SOURCE_CODE, LABEL);
	}

	void			SLStage::release_glObj()
	{
		if (ID() != 0)
		{
			glDeleteShader(ID);
			ID = 0;
		}
	}

	/*
	void						Stage::expand(					Parser&							parser,
																SourceCode&						expandedCode,
																Externals&						includedFiles)
	{
		// Check file lines one by one with main parser.
		while (parser.parse_line())
		{
			// This flag indicates whether or not current line will be copied into the buffer.
			bool	addLine = true;

			Parser	lineParser(parser.substring());

			// Any parsed word has to be compared with keywords.
			if (lineParser.parse_word())
			{
				// Shader includes external components and current line(the line with '#include') will be replaced by them.
				if (lineParser.substring() == "#include")
				{
					// Line with this keyword has to be skipped.
					addLine = false;

					// All characters after '#include' are considered to be file/component name.
					if (lineParser.parse_word() == false)
						throw GeneralException(this, __LINE__, "You must put file path or component name after '#include' keyword.");

					std::string file = lineParser.string();

					// Add new code only if it wasn't already included.
					if(includedFiles.find(file) == includedFiles.end())
					{
						Parser::Source source(file);

						expand(Parser(source), expandedCode, includedFiles);
					}
				}
			}

			if (addLine)
			{
				expandedCode += lineParser.range().to_str() + '\n';
			}
		}
	}

	glsl::Struct*				Stage::register_struct_type(	const glsl::Type::Name&			typeName,
																glsl::Type::Map&				customTypes) const
	{
		// Check default types.
		if(glsl::get_default_type(typeName))
			throw GeneralException(this, __LINE__, "Struct name cannot be the same as default uniform type: " + typeName);

		auto result = customTypes.emplace(typeName, nullptr);
		if(!result.second)
			throw GeneralException(this, __LINE__, "Struct with this name was already created: " + typeName);

		auto* newStruct = new glsl::Struct(typeName);
		result.first->second.reset(newStruct);
		return newStruct;
	}

	std::shared_ptr<const glsl::Type>	Stage::find_uniform_type(		const glsl::Type::Name&			typeName,
																const glsl::Type::Map&			customTypes) const
	{
		auto defaultType = glsl::get_default_type(typeName);
		if(defaultType)
			return defaultType;

		auto iCustom = customTypes.find(typeName);
		if(iCustom != customTypes.end()) 
			return iCustom->second;

		return nullptr;
	}

	void						Stage::unwrap_uniform(			const std::string&				prefix,
																const glsl::Variable&			uniform)
	{
		if(uniform.type()->signature() == glsl::Type::eSTRUCT)
		{
			auto* structUniform = static_cast<const glsl::Struct*>(uniform.type().get());
					
			std::string uniformPrefix = prefix + uniform.string() + ".";

			for(auto& iMember : structUniform->members())
				unwrap_uniform(uniformPrefix, iMember);
		}
		//else // Add unwrapped uniform to the array.
		//{
		//	streams->emplace_back(	ExternalStream::eUNIFORM, 
		//							ExternalStream::UNKNOWN_LOCATION,
		//							uniform.type(), 
		//							prefix + uniform.name(), 
		//							uniform.numElements);
		//}
	}

	glsl::Variable				Stage::parse_variable(			const std::shared_ptr<const glsl::Type>&	variableType,
																Parser&							parser)
	{
		//Uniform name can be followed by semicolon or [X] expresion so it must be parsed separately.
		//exemplary uniform lines:
		//	uniform vec3 normal;
		//	uniform vec2 offsets[5];

		std::string name;
		uint32_t	arrayElements = 1;

		// new parsing operation starts right after uniform type word
		Parser arrayParser(parser.carriage(), parser.range().end());

		// if given range contains opening brackets it may be valid array of uniforms
		if (arrayParser.parse_while('['))
		{
			// we have to take care of diffrent formatting possibilities, eg. 'uniform vec2 offsets  [ x ]'
			// this parser will remove any white spaces from string 
			Parser nameParser(arrayParser.carriage(), arrayParser.substring().end());

			name = nameParser.string();

			if (!arrayParser.parse_block('[', ']'))
				throw GeneralException(this, __LINE__, "Unable to parse uniform array.");

			// remove white spaces from array brackets
			Parser numParser(arrayParser.substring());
			if (!numParser.parse_word())
				throw GeneralException(this, __LINE__, "Unable to parse value from uniform array.");

			arrayElements = to_ulong(numParser.string());
		}
		else
		{
			// single uniform variable
			if (!parser.parse_word())
				throw GeneralException(this, __LINE__, "Unable to parse uniform name.");

			name = parser.string();
		}

		return glsl::Variable(variableType, name, arrayElements);
	}

	void						Stage::parse_struct(			Parser&							parser,
																glsl::Type::Map&				customTypes)
	{
		//exemplary structs:
		//struct Test1{
		//	float	a;
		//	int		b;
		//};
		//struct Test2
		//{
		//	float	c;
		//	int		d;
		//};

		// Parse characters until '{' character.
		if (!parser.parse_while('{'))
			throw GeneralException(this, __LINE__, "Fail to parse struct name.");

		// Remove white spaces from substring.
		Parser structName(parser.substring());
		if (!structName.parse_word())
			throw GeneralException(this, __LINE__, "Struct has no name.");

		glsl::Struct* newStruct = register_struct_type(structName.string(), customTypes);

		// parse whole struct block
		if (!parser.parse_block('{', '}'))
			throw GeneralException(this, __LINE__, "Fail to parse struct body.");

		Parser structBody(parser.substring());
		while (structBody.parse_word())
		{
			if(auto variableType = find_uniform_type(structBody.string(), customTypes))
			{
				structBody.parse_while(';');

				newStruct->add_member(parse_variable(variableType, Parser(structBody.substring())));
			}
		}
	}

	void						Stage::parse_code(				Parser&							parser,
																glsl::Type::Map&				customTypes)
	{
		// Check file lines one by one with main parser.
		while (parser.parse_word())
		{
			if (parser.substring() == "uniform")
			{
				// Parsing starts right after 'uniform' keyword and has to end with semicolon.
				parser.parse_while(';');
				
				Parser uniformParser(parser.substring());

				if (!uniformParser.parse_word())
					throw GeneralException(this, __LINE__, "Unable to parse uniform type.");

				if(auto variableType = find_uniform_type(uniformParser.string(), customTypes))
				{
					unwrap_uniform("", parse_variable(variableType, uniformParser));
				}
				else
				{
					throw GeneralException(this, __LINE__, "Unknow uniform type: " + uniformParser.string());
				}

				// Move past the semicolon.
				parser.skip_char();
			}
			else if (parser.substring() == "struct")
			{
				parse_struct(parser, customTypes);
			}
		}
	}
	*/
}