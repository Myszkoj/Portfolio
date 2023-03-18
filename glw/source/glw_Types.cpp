#include "..//include/glw_Types.h"


namespace glw
{
	struct TypeMapWrapper
	{
		TypeMap typeMap;

		CLASS_CTOR TypeMapWrapper()
		{
			typeMap[GL_TRUE]			= {sizeof(GLboolean),	true};
			typeMap[GL_FALSE]			= {sizeof(GLboolean),	true};
			typeMap[GL_BYTE]			= {sizeof(GLbyte),		true};
			typeMap[GL_UNSIGNED_BYTE]	= {sizeof(GLubyte),		true};
			typeMap[GL_SHORT]			= {sizeof(GLshort),		true};
			typeMap[GL_UNSIGNED_SHORT]	= {sizeof(GLushort),	true};
			typeMap[GL_INT]				= {sizeof(GLint),		true};
			typeMap[GL_UNSIGNED_INT]	= {sizeof(GLuint),		true};
			typeMap[GL_HALF_FLOAT]		= {sizeof(GLhalf),		false};
			typeMap[GL_FLOAT]			= {sizeof(GLfloat),		false};
			typeMap[GL_DOUBLE]			= {sizeof(GLdouble),	false};
			
		}
	};

	TypeDescription get_typeDescription(const GLenum BASE)
	{
		static const TypeMapWrapper wrapper;
		auto it = wrapper.typeMap.find(BASE);
		return it != wrapper.typeMap.end() ? it->second : TypeDescription();
	}


	struct DefaultTypes
	{
	public: // data
		SLType::Map map;

	public: // functions
		CLASS_CTOR DefaultTypes()
		{
			map["float"]			= std::make_unique<Float>();
			map["double"]			= std::make_unique<Double>();
			map["int"]				= std::make_unique<Int>();
			map["uint"]				= std::make_unique<Uint>();
																		
			map["vec2"]				= std::make_unique<Vec2>();
			map["ivec2"]			= std::make_unique<IVec2>();
			map["uvec2"]			= std::make_unique<UVec2>();

			map["vec3"]				= std::make_unique<Vec3>();
			map["ivec3"]			= std::make_unique<IVec3>();
			map["uvec3"]			= std::make_unique<UVec3>();

			map["vec4"]				= std::make_unique<Vec4>();
			map["ivec4"]			= std::make_unique<IVec4>();
			map["uvec4"]			= std::make_unique<UVec4>();
					
			map["mat2"]				= std::make_unique<Mat2>();
			map["mat3"]				= std::make_unique<Mat3>();
			map["mat4"]				= std::make_unique<Mat4>();

			map["sampler1D"]		= std::make_unique<Sampler1D>();
			map["isampler1D"]		= std::make_unique<ISampler1D>();
			map["usampler1D"]		= std::make_unique<USampler1D>();
														
			map["sampler2D"]		= std::make_unique<Sampler2D>();
			map["isampler2D"]		= std::make_unique<ISampler2D>();
			map["usampler2D"]		= std::make_unique<USampler2D>();

			map["sampler3D"]		= std::make_unique<Sampler3D>();
			map["isampler3D"]		= std::make_unique<ISampler3D>();
			map["usampler3D"]		= std::make_unique<USampler3D>();

			map["samplerCube"]		= std::make_unique<SamplerCube>();
			map["isamplerCube"]		= std::make_unique<ISamplerCube>();
			map["usamplerCube"]		= std::make_unique<USamplerCube>();
																			
			map["samplerBuffer"]	= std::make_unique<SamplerBuffer>();
			map["isamplerBuffer"]	= std::make_unique<ISamplerBuffer>();
			map["usamplerBuffer"]	= std::make_unique<USamplerBuffer>();
		}
	};

	SLType*			get_default_type(	const SLType::Name& typeName)
	{
		static DefaultTypes defaultTypes;
		auto it = defaultTypes.map.find(typeName);
		return (it != defaultTypes.map.end()) ? it->second.get() : nullptr;
	}
}