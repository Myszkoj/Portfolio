#include "..//include/glw_SLProgram.h"


namespace glw
{
	/* STATIC */ GLuint		SLProgram::sm_currentProgramID = 0;

//=====> SLProgram private: // lifecycle
	CLASS_CTOR				SLProgram::SLProgram(				const dpl::Ownership&		OWNERSHIP,
																const Binding& 				BINDING)
		: Resource(OWNERSHIP, BINDING)
		, m_id(0)
		, bNeedsRebuild(false)
	{

	}

//=====> SLProgram public: // lifecycle
	CLASS_CTOR				SLProgram::SLProgram(				const dpl::Ownership&		OWNERSHIP,
																const Binding& 				BINDING,
																const Design&				DESIGN)
		: SLProgram(OWNERSHIP, BINDING)
	{
		DESIGN(*this);
	}

	CLASS_CTOR				SLProgram::SLProgram(				const dpl::Ownership&		OWNERSHIP,
																SLProgram&&					other) noexcept
		: Resource(OWNERSHIP, std::move(other))
		, MyVSH(std::move(other))
		, MyFSH(std::move(other))
		, MyGSH(std::move(other))
		, m_id(other.m_id)
		, bNeedsRebuild(other.bNeedsRebuild)
	{
		other.m_id			= 0;
		other.bNeedsRebuild = true;
	}

	CLASS_DTOR				SLProgram::~SLProgram()
	{
		release_glObj();
	}

	SLProgram&				SLProgram::operator=(				dpl::Swap<SLProgram>		other)
	{
		Resource::operator=(dpl::Swap(*other));
		MyVSH::operator=(dpl::Swap<MyVSH>(*other));
		MyFSH::operator=(dpl::Swap<MyFSH>(*other));
		MyGSH::operator=(dpl::Swap<MyGSH>(*other));
		std::swap(m_id, other->m_id);
		std::swap(bNeedsRebuild, other->bNeedsRebuild);
		return *this;
	}

//=====> SLProgram public: // functions
	bool					SLProgram::bind()
	{
		if(bNeedsRebuild)
		{
			rebuild();
			bNeedsRebuild = false;
		}

		if(sm_currentProgramID != m_id)
		{
			glUseProgram(m_id);
			sm_currentProgramID = m_id;
			return true;
		}

		return false;
	}

	bool					SLProgram::bind_default()
	{
		if(sm_currentProgramID != 0)
		{
			glUseProgram(0);
			sm_currentProgramID = 0;
			return true;
		}

		return false;
	}

	void					SLProgram::set_attribute_location(	const std::string&			NAME,
																const int32_t				LOCATION)
	{
		glBindAttribLocation(m_id, LOCATION, NAME.c_str());
		validate_opengl(__FILE__, __LINE__, "Program[" + this->get_label()  + "]: Failed to set attribute location: " + NAME);
	}

	int32_t					SLProgram::get_attribute_location(	const std::string&			NAME) const
	{
		int32_t location = glGetAttribLocation(m_id, NAME.c_str());
		validate_opengl(__FILE__, __LINE__, "Program[" + this->get_label()  + "]: Failed to get attribute location: " + NAME);
		if(location == -1) throw dpl::GeneralException(this, __LINE__, "Program[" + this->get_label()  + "]: Failed to find attribute with given name: " + NAME);
		return location;
	}

	void					SLProgram::get_attribute_list(		std::vector<std::string>&	names)
	{
		GLint			count = 0;

		GLint			size = 0; // size of the variable
		GLenum			type = GL_NONE; // type of the variable (float, vec3 or mat4, etc)

		const GLsizei	bufSize = 32; // maximum name length
		GLchar			attributeName[bufSize]; // variable name in GLSL
		GLsizei			length = 0; // name length

		glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTES, &count);
		printf("Active IAttributes: %d\n", count);

		for (GLint i = 0; i < count; i++)
		{
			glGetActiveAttrib(m_id, (GLuint)i, bufSize, &length, &size, &type, attributeName);

			names.emplace_back("[" + std::to_string(i) + "] Name: " + std::string(attributeName) + " Type: " + std::to_string(type));
		}
	}

	void					SLProgram::set_fragData_location(	const std::string&			NAME,
																const int32_t				LOCATION)
	{
		glBindFragDataLocation(m_id, LOCATION, NAME.c_str());
		validate_opengl(__FILE__, __LINE__, "Program[" + this->get_label()  + "]: Failed to set fragment data location: " + NAME);
	}

	int32_t					SLProgram::get_fragData_location(	const std::string&			NAME) const
	{
		int32_t location = glGetFragDataLocation(m_id, NAME.c_str());
		validate_opengl(__FILE__, __LINE__, "Program[" + this->get_label()  + "]: Failed to get fragment data location: " + NAME);
		if(location == -1)
			throw dpl::GeneralException(this, __LINE__, "Program[" + this->get_label()  + "]: Failed to find fragment data with given name: " + NAME);

		return location;
	}

	int32_t					SLProgram::get_uniform_location(	const std::string&			NAME) const
	{
		int32_t location = glGetUniformLocation(m_id, NAME.c_str());
		validate_opengl(__FILE__, __LINE__, "Program[" + this->get_label() + "]: Failed to get uniform location: " + NAME);
		if(location == -1) throw dpl::GeneralException(this, __LINE__, "Program[" + this->get_label() + "]: Failed to find uniform with given name: " + NAME);
		return location;
	}

	void					SLProgram::get_uniform_list(		std::vector<std::string>&	names)
	{
		GLint			count = 0;

		GLint			size = 0; // size of the variable
		GLenum			type = GL_NONE; // type of the variable (float, vec3 or mat4, etc)

		const GLsizei	bufSize = 32; // maximum name length
		GLchar			uniformName[bufSize]; // variable name in GLSL
		GLsizei			length = 0; // name length

		glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &count);
		printf("Active Uniforms: %d\n", count);

		for (GLint i = 0; i < count; i++)
		{
			glGetActiveUniform(m_id, (GLuint)i, bufSize, &length, &size, &type, uniformName);

			names.emplace_back("[" + std::to_string(i) + "] Name: " + std::string(uniformName) + " Type: " + std::to_string(type));
		}
	}

//=====> SLProgram private: // functions
	void					SLProgram::generate()
	{
		if(m_id != 0)
			throw dpl::GeneralException(this, __LINE__, "Program already generated.");

		m_id = glCreateProgram();
		validate_opengl(__FILE__, __LINE__, "Fail to generate shader program.");
	}

	void					SLProgram::attach_shader(			SLStage&					shader)
	{
		glAttachShader(m_id, shader.ID());
		validate_opengl(__FILE__, __LINE__, "Could not attach shader: " + std::string(shader.get_name()));

		using	NonUniforms = std::unordered_set<std::string>;
				NonUniforms map;

		shader.for_each_external_transfer([&](const ExternalSLTransfer& transfer)
		{
			if(transfer.qualifier != ExternalSLTransfer::eUNIFORM)
			{
				transfer.bind_program(*this);
			}
		});
	}

	void					SLProgram::attach_shaders()
	{
		if(MyVSH::has_resource())	attach_shader(MyVSH::get_resource());
		if(MyFSH::has_resource())	attach_shader(MyFSH::get_resource());
		if(MyGSH::has_resource())	attach_shader(MyGSH::get_resource());
	}

	void					SLProgram::link_shaders()
	{
		glLinkProgram(m_id);

		// check for errors
		GLint programLoaded = GL_FALSE;
		glGetProgramiv(m_id, GL_LINK_STATUS, &programLoaded);

		if (programLoaded != GL_TRUE)
		{
			std::string shaderNames;

			if(MyVSH::has_resource())
			{
				shaderNames += "\t" + std::string(MyVSH::get_resource().get_name()) + "\n";
			}

			if(MyFSH::has_resource())
			{
				shaderNames += "\t" + std::string(MyFSH::get_resource().get_name()) + "\n";
			}

			if(MyGSH::has_resource())
			{
				shaderNames += "\t" + std::string(MyGSH::get_resource().get_name()) + "\n";
			}

			throw dpl::GeneralException(this, __LINE__, "Fail to link shaders: " + shaderNames + "--> error: " + get_program_info());
		}
	}

	void					SLProgram::release_glObj()
	{
		if(m_id != 0)
		{
			if(sm_currentProgramID == m_id)
				bind_default();

			glDeleteProgram(m_id);
			m_id = 0;
		}
	}

	std::string				SLProgram::get_program_info()
	{
		int infoLogLength	= 0;
		int maxLength		= 0;

		glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &maxLength);

		std::unique_ptr<char[]> infoLog(new char[maxLength]);

		glGetProgramInfoLog(m_id, maxLength, &infoLogLength, infoLog.get());

		return std::string(infoLog.get(), infoLogLength);
	}

	void					SLProgram::rebuild()
	{
		generate();
		attach_shaders();
		link_shaders();
	}
}