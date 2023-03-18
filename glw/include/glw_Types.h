#pragma once


#include <dpl_TypeTraits.h>
#include "glw_Utilities.h"
#include <typeindex>


#pragma warning( disable : 26451)
#pragma warning( disable : 26812)

namespace glw
{
	struct TypeDescription
	{
		uint32_t	bytes		= 0;
		bool		bInteger	= false;
	};

	using TypeMap = std::unordered_map<GLenum, TypeDescription>;
	TypeDescription get_typeDescription(const GLenum BASE);

	template<GLenum BASE, uint32_t COMPONENTS>
	struct TypeTranslator{};

	template<> struct TypeTranslator<GL_FLOAT,			1>{	using Type = float;		static const GLenum BASE = GL_FLOAT;		static const GLenum GL = GL_R32F;		static const GLenum FORMAT = GL_RED;};
	template<> struct TypeTranslator<GL_DOUBLE,			1>{	using Type = double;	static const GLenum BASE = GL_DOUBLE;		static const GLenum GL = GL_RG32F;		static const GLenum FORMAT = GL_RED;};
	template<> struct TypeTranslator<GL_INT,			1>{	using Type = int32_t;	static const GLenum BASE = GL_INT;			static const GLenum GL = GL_R32I;		static const GLenum FORMAT = GL_RED_INTEGER;};
	template<> struct TypeTranslator<GL_UNSIGNED_INT,	1>{	using Type = uint32_t;	static const GLenum BASE = GL_UNSIGNED_INT;	static const GLenum GL = GL_R32UI;		static const GLenum FORMAT = GL_RED_INTEGER;};

	template<> struct TypeTranslator<GL_FLOAT,			2>{	using Type = float;		static const GLenum BASE = GL_FLOAT;		static const GLenum GL = GL_RG32F;		static const GLenum FORMAT = GL_RG;};
	template<> struct TypeTranslator<GL_DOUBLE,			2>{	using Type = double;	static const GLenum BASE = GL_DOUBLE;		static const GLenum GL = GL_RGBA32F;	static const GLenum FORMAT = GL_RG;};
	template<> struct TypeTranslator<GL_INT,			2>{	using Type = int32_t;	static const GLenum BASE = GL_INT;			static const GLenum GL = GL_RG32I;		static const GLenum FORMAT = GL_RG_INTEGER;};
	template<> struct TypeTranslator<GL_UNSIGNED_INT,	2>{	using Type = uint32_t;	static const GLenum BASE = GL_UNSIGNED_INT;	static const GLenum GL = GL_RG32UI;		static const GLenum FORMAT = GL_RG_INTEGER;};

	template<> struct TypeTranslator<GL_FLOAT,			3>{	using Type = float;		static const GLenum BASE = GL_FLOAT;		static const GLenum GL = GL_RGB32F;		static const GLenum FORMAT = GL_RGB;};
	template<> struct TypeTranslator<GL_INT,			3>{	using Type = int32_t;	static const GLenum BASE = GL_INT;			static const GLenum GL = GL_RGB32I;		static const GLenum FORMAT = GL_RGB_INTEGER;};
	template<> struct TypeTranslator<GL_UNSIGNED_INT,	3>{	using Type = uint32_t;	static const GLenum BASE = GL_UNSIGNED_INT;	static const GLenum GL = GL_RGB32UI;	static const GLenum FORMAT = GL_RGB_INTEGER;};

	template<> struct TypeTranslator<GL_FLOAT,			4>{	using Type = float;		static const GLenum BASE = GL_FLOAT;		static const GLenum GL = GL_RGBA32F;	static const GLenum FORMAT = GL_RGBA;};
	template<> struct TypeTranslator<GL_INT,			4>{	using Type = int32_t;	static const GLenum BASE = GL_INT;			static const GLenum GL = GL_RGBA32I;	static const GLenum FORMAT = GL_RGBA_INTEGER;};
	template<> struct TypeTranslator<GL_UNSIGNED_INT,	4>{	using Type = uint32_t;	static const GLenum BASE = GL_UNSIGNED_INT;	static const GLenum GL = GL_RGBA32UI;	static const GLenum FORMAT = GL_RGBA_INTEGER;};

/*################################# GLSL TYPE INTERFACE #################################*/

	/*
		Interface for glsl type wrappers.
	*/
	class	SLType : public dpl::Subject<SLType>
	{
	public: // subtypes
		using	Name	= std::string;
		using	Map		= std::unordered_map<Name, std::unique_ptr<SLType>>; // Type dictionary.

		enum	Signature
		{
			eSCALAR,
			eVECTOR,
			eMATRIX,
			eSAMPLER
		};

	public: // data
		dpl::ReadOnly<Name,			SLType> name;
		dpl::ReadOnly<GLenum,		SLType> glBase;
		dpl::ReadOnly<uint32_t,		SLType> columns;
		dpl::ReadOnly<uint32_t,		SLType> rows;
		dpl::ReadOnly<Signature,	SLType> signature;

	public: // lifecycle
		CLASS_CTOR				SLType(	const Name&		NAME,
										const GLenum	GL_BASE,
										const uint32_t	COLUMNS,
										const uint32_t	ROWS,
										const Signature	SIGNATURE)
			: name(NAME)
			, glBase(GL_BASE)
			, columns(COLUMNS)
			, rows(ROWS)
			, signature(SIGNATURE)
		{
			
		}

	public: // interface
		/*
		*	Returns type_index of the data type represented by this class.
		*/
		virtual std::type_index get_id() const = 0;
	};


	template<typename T>
	concept is_glsl_type = std::is_base_of_v<SLType, T>;


	SLType*	get_default_type(	const SLType::Name& TYPE_NAME);



/*################################# SCALAR TYPES #################################*/

	template<GLenum GL_BASE>
	class	Scalar : public SLType
	{
	public: // subtypes
		using Format		= TypeTranslator<GL_BASE, 1>;
		using ComponentT	= typename Format::Type;
		using DataT			= ComponentT;

	protected: // lifecycle
		CLASS_CTOR				Scalar(		const Name&		NAME)
			: SLType(NAME, GL_BASE, 1, 1, Signature::eSCALAR)
		{

		}

	public: // Type implementation
		virtual std::type_index get_id() const final override
		{
			return std::type_index(typeid(DataT));
		}
	};


	/*
		Defines scalar type class.
	*/
	#define GLW_SCALAR_TYPE(typeName, className, GL_BASE, func)				\
	className : public Scalar<GL_BASE>{										\
	public: using DataT = typename Scalar<GL_BASE>::DataT;					\
	public: static constexpr char NAME[] = typeName;						\
	public:																	\
		className()	: Scalar(typeName){}									\
		inline void update_uniform(			GLint			ID,				\
											const DataT&	value,			\
											uint32_t		offset) const	\
		{ func(ID + offset, 1, &value); }									\
		inline void update_uniform_array(	GLint			ID,				\
											const DataT*	first,			\
											uint32_t		size) const		\
		{ func(ID, size, first); }											\
	}



	class GLW_SCALAR_TYPE("float",	Float,	GL_FLOAT,			glUniform1fv);
	class GLW_SCALAR_TYPE("double",	Double,	GL_DOUBLE,			glUniform1dv);
	class GLW_SCALAR_TYPE("int",	Int,	GL_INT,				glUniform1iv);
	class GLW_SCALAR_TYPE("uint",	Uint,	GL_UNSIGNED_INT,	glUniform1uiv);


/*################################# VECTOR TYPES #################################*/

	template<uint32_t COMPONENTS, GLenum GL_BASE>
	class	Vector : public SLType
	{
	public: // subtypes
		using Format		= TypeTranslator<GL_BASE, COMPONENTS>;
		using ComponentT	= typename Format::Type;
		using DataT			= glm::vec<COMPONENTS, ComponentT, glm::highp>;

	protected: // lifecycle
		CLASS_CTOR				Vector(	const Name&	NAME)
			: SLType(NAME, GL_BASE, COMPONENTS, 1, Signature::eVECTOR)
		{

		}

	public: // Type implementation
		virtual std::type_index get_id() const final override
		{
			return std::type_index(typeid(DataT));
		}
	};



	/*
		Defines vector type class.
	*/
	#define GLW_VECTOR_TYPE(typeName, className, components, GL_BASE, func)	\
	className : public Vector<components, GL_BASE>{							\
	public: using DataT = typename Vector<components, GL_BASE>::DataT;		\
	public: static constexpr char NAME[] = typeName;						\
	public:																	\
		className()	: Vector(typeName){}									\
		inline void update_uniform(			GLint			ID,				\
											const DataT&	value,			\
											uint32_t		offset) const	\
		{ func(ID + offset, 1, &value[0]); }								\
		inline void update_uniform_array(	GLint			ID,				\
											const DataT*	first,			\
											uint32_t		size) const		\
		{ func(ID, size, &(*first)[0]); }									\
	}


	class GLW_VECTOR_TYPE("vec2",	Vec2,	2, GL_FLOAT,			glUniform2fv);
	class GLW_VECTOR_TYPE("ivec2",	IVec2,	2, GL_INT,				glUniform2iv);
	class GLW_VECTOR_TYPE("uvec2",	UVec2,	2, GL_UNSIGNED_INT,		glUniform2uiv);

	class GLW_VECTOR_TYPE("vec3",	Vec3,	3, GL_FLOAT,			glUniform3fv);
	class GLW_VECTOR_TYPE("ivec3",	IVec3,	3, GL_INT,				glUniform3iv);
	class GLW_VECTOR_TYPE("uvec3",	UVec3,	3, GL_UNSIGNED_INT,		glUniform3uiv);

	class GLW_VECTOR_TYPE("vec4",	Vec4,	4, GL_FLOAT,			glUniform4fv);
	class GLW_VECTOR_TYPE("ivec4",	IVec4,	4, GL_INT,				glUniform4iv);
	class GLW_VECTOR_TYPE("uvec4",	UVec4,	4, GL_UNSIGNED_INT,		glUniform4uiv);
		
/*################################# MATRIX TYPES #################################*/

	template<uint32_t COLUMNS, uint32_t	ROWS, GLenum GL_BASE>
	class	Matrix : public SLType
	{
	public: // subtypes
		using Format		= TypeTranslator<GL_BASE, ROWS>;
		using ComponentT	= typename Format::Type;
		using DataT			= glm::mat<COLUMNS, ROWS, ComponentT, glm::highp>;

	protected: // lifecycle
		CLASS_CTOR				Matrix(	const Name&		NAME)
			: SLType(NAME, GL_FLOAT, COLUMNS, ROWS, Signature::eMATRIX)
		{

		}

	public: // Type implementation
		virtual std::type_index get_id() const final override
		{
			return std::type_index(typeid(DataT));
		}
	};



	/*
		Defines matrix type class.
	*/
	#define GLW_MATRIX_TYPE(typeName, className, COLUMNS, ROWS, GL_BASE, func)	\
	className : public Matrix<COLUMNS, ROWS, GL_BASE>{							\
	public: using DataT = typename Matrix<COLUMNS, ROWS, GL_BASE>::DataT;		\
	public: static constexpr char NAME[] = typeName;							\
	public:																		\
		className()	: Matrix(typeName){}										\
		inline void update_uniform(			GLint			ID,					\
											const DataT&	value,				\
											uint32_t		offset) const		\
		{ func(ID + offset, 1, GL_FALSE, &value[0][0]); }						\
		inline void update_uniform_array(	GLint			ID,					\
											const DataT*	first,				\
											uint32_t		size) const			\
		{ func(ID, size, GL_FALSE, &(*first)[0][0]); }							\
	}


	class GLW_MATRIX_TYPE("mat2",	Mat2, 2, 2, GL_FLOAT,	glUniformMatrix2fv);
	class GLW_MATRIX_TYPE("mat3",	Mat3, 3, 3, GL_FLOAT,	glUniformMatrix3fv);
	class GLW_MATRIX_TYPE("mat4",	Mat4, 4, 4, GL_FLOAT,	glUniformMatrix4fv);

/*################################# SAMPLER TYPES #################################*/

	template<GLenum GL_BASE>
	class	Sampler : public SLType
	{
	public: // subtypes
		using Format		= TypeTranslator<GL_BASE, 1>;
		using ComponentT	= typename Format::Type;
		using DataT			= int32_t;

	public: // lifecycle
		CLASS_CTOR				Sampler(	const Name&	NAME)
			: SLType(NAME, GL_NONE, 0, 0, Signature::eSAMPLER)
		{

		}

	public: // Type implementation
		virtual std::type_index get_id() const final override
		{
			return std::type_index(typeid(DataT));
		}
	};



	/*
		Defines sampler type class.
	*/
	#define GLW_SAMPLER_TYPE(typeName, className, GL_BASE)					\
	className : public Sampler<GL_BASE>{									\
	public: using DataT = typename Sampler<GL_BASE>::DataT;					\
	public: static constexpr char NAME[] = typeName;						\
	public:																	\
		className()	: Sampler(typeName){}									\
		inline void update_uniform(			GLint			ID,				\
											DataT			value,			\
											uint32_t		offset) const	\
		{ glUniform1iv(ID + offset, 1, &value); }							\
		inline void update_uniform_array(	GLint			ID,				\
											const DataT*	first,			\
											uint32_t		size) const		\
		{ glUniform1iv(ID, size, first); }									\
	}



	class GLW_SAMPLER_TYPE("sampler1D",			Sampler1D,			GL_FLOAT);
	class GLW_SAMPLER_TYPE("isampler1D",		ISampler1D,			GL_INT);
	class GLW_SAMPLER_TYPE("usampler1D",		USampler1D,			GL_UNSIGNED_INT);

	class GLW_SAMPLER_TYPE("sampler2D",			Sampler2D,			GL_FLOAT);
	class GLW_SAMPLER_TYPE("isampler2D",		ISampler2D,			GL_INT);
	class GLW_SAMPLER_TYPE("usampler2D",		USampler2D,			GL_UNSIGNED_INT);

	class GLW_SAMPLER_TYPE("sampler3D",			Sampler3D,			GL_FLOAT);
	class GLW_SAMPLER_TYPE("isampler3D",		ISampler3D,			GL_INT);
	class GLW_SAMPLER_TYPE("usampler3D",		USampler3D,			GL_UNSIGNED_INT);

	class GLW_SAMPLER_TYPE("sampler2DArray",	Sampler2DArray,		GL_FLOAT);
	class GLW_SAMPLER_TYPE("isampler2DArray",	ISampler2DArray,	GL_INT);
	class GLW_SAMPLER_TYPE("usampler2DArray",	USampler2DArray,	GL_UNSIGNED_INT);

	class GLW_SAMPLER_TYPE("samplerCube",		SamplerCube,		GL_FLOAT);
	class GLW_SAMPLER_TYPE("isamplerCube",		ISamplerCube,		GL_INT);
	class GLW_SAMPLER_TYPE("usamplerCube",		USamplerCube,		GL_UNSIGNED_INT);

	class GLW_SAMPLER_TYPE("samplerBuffer",		SamplerBuffer,		GL_FLOAT);
	class GLW_SAMPLER_TYPE("isamplerBuffer",	ISamplerBuffer,		GL_INT);
	class GLW_SAMPLER_TYPE("usamplerBuffer",	USamplerBuffer,		GL_UNSIGNED_INT);


	template<typename T>struct Sampler1DSelector{};
	template<> struct Sampler1DSelector<float>{			using Type = Sampler1D;};
	template<> struct Sampler1DSelector<double>{		using Type = Sampler1D;};
	template<> struct Sampler1DSelector<int32_t>{		using Type = ISampler1D;};
	template<> struct Sampler1DSelector<uint32_t>{		using Type = USampler1D;};

	template<typename T>struct Sampler2DSelector{};
	template<> struct Sampler2DSelector<float>{			using Type = Sampler2D;};
	template<> struct Sampler2DSelector<double>{		using Type = Sampler2D;};
	template<> struct Sampler2DSelector<int32_t>{		using Type = ISampler2D;};
	template<> struct Sampler2DSelector<uint32_t>{		using Type = USampler2D;};

	template<typename T>struct Sampler3DSelector{};
	template<> struct Sampler3DSelector<float>{			using Type = Sampler3D;};
	template<> struct Sampler3DSelector<double>{		using Type = Sampler3D;};
	template<> struct Sampler3DSelector<int32_t>{		using Type = ISampler3D;};
	template<> struct Sampler3DSelector<uint32_t>{		using Type = USampler3D;};

	template<typename T>struct Sampler2DArraySelector{};
	template<> struct Sampler2DArraySelector<float>{	using Type = Sampler2DArray;};
	template<> struct Sampler2DArraySelector<double>{	using Type = Sampler2DArray;};
	template<> struct Sampler2DArraySelector<int32_t>{	using Type = ISampler2DArray;};
	template<> struct Sampler2DArraySelector<uint32_t>{	using Type = USampler2DArray;};

	template<typename T>struct SamplerCubeSelector{};
	template<> struct SamplerCubeSelector<float>{		using Type = SamplerCube;};
	template<> struct SamplerCubeSelector<double>{		using Type = SamplerCube;};
	template<> struct SamplerCubeSelector<int32_t>{		using Type = ISamplerCube;};
	template<> struct SamplerCubeSelector<uint32_t>{	using Type = USamplerCube;};

	template<typename T> struct SamplerBufferSelector{};
	template<> struct SamplerBufferSelector<float>{		using Type = SamplerBuffer;};
	template<> struct SamplerBufferSelector<double>{	using Type = SamplerBuffer;};
	template<> struct SamplerBufferSelector<int32_t>{	using Type = ISamplerBuffer;};
	template<> struct SamplerBufferSelector<uint32_t>{	using Type = USamplerBuffer;};
}