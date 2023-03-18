#pragma once


#include <dpl_TypeTraits.h>
#include "glw_Buffer.h"

// vertex attribute
namespace glw
{
	template<typename T>
	class	Attribute : public dpl::NamedType<T>{};

	template<typename T>
	concept is_Attribute = std::is_base_of_v<Attribute<T>, T>;

	template<typename T>
	struct	IsAttribute
	{
		static const bool value = is_Attribute<T>;
	};

	template<typename AttributeTypeListT>
	concept	is_AttributeTypeList	= dpl::is_TypeList<AttributeTypeListT> 
									&& AttributeTypeListT::SIZE > 0
									&& AttributeTypeListT::ALL_UNIQUE 
									&& AttributeTypeListT::template all<IsAttribute>();
}

// vertex
namespace glw
{
	template<is_AttributeTypeList Ats>
	class Vertex
	{
	public: // subtypes
		using ATTRIBUTE_TYPES	= Ats;
		using Data_t			= typename ATTRIBUTE_TYPES::DataPack;

	public: // constants
		static const uint32_t NUM_ATTRIBUTES = ATTRIBUTE_TYPES::SIZE;

	public: // data
		Data_t data;

	public: // functions
		template<is_Attribute T>
		static constexpr bool		has()
		{
			return ATTRIBUTE_TYPES::template has_type<T>();
		}

		template<is_Attribute T>
		static constexpr size_t		offset_of()
		{
			return ATTRIBUTE_TYPES::template data_byte_offset<T>();
		}

		template<is_Attribute T>
		inline T&					get()
		{
			return std::get<T>(data);
		}

		template<is_Attribute T>
		inline const T&				get() const
		{
			return std::get<T>(data);
		}

		template<is_Attribute T>
		inline void					set(const T& VALUE)
		{
			Vertex::get<T>() = VALUE;
		}
	};
}

// demo
namespace glw
{
	class Demo_Attribute0 : public Attribute<Demo_Attribute0>
	{
	public: // data
		float x;
		float y;
	};

	class Demo_Attribute1 : public Attribute<Demo_Attribute1>
	{
	public: // data
		float x;
	};

	class Demo_Attribute2 : public Attribute<Demo_Attribute2>
	{
	public: // data
		float x;
		float y;
		float z;
	};

	using Demo_Vertex = Vertex<dpl::TypeList<Demo_Attribute0, Demo_Attribute1, Demo_Attribute2>>;


	void vertex_showcase()
	{
		Demo_Vertex vertex;
					vertex.set(Demo_Attribute0());

		const size_t OFFSET0 = Demo_Vertex::offset_of<Demo_Attribute0>();
		const size_t OFFSET1 = Demo_Vertex::offset_of<Demo_Attribute1>();
		const size_t OFFSET2 = Demo_Vertex::offset_of<Demo_Attribute2>();

		const auto	TOTAL_SIZE	= sizeof(vertex);
		const auto& ATT0		= vertex.get<Demo_Attribute0>();
		const auto& ATT1		= vertex.get<Demo_Attribute1>();
		const auto& ATT2		= vertex.get<Demo_Attribute2>();

		const size_t DIFF0 = (const char*)&ATT0 - (const char*)&vertex;
		const size_t DIFF1 = (const char*)&ATT1 - (const char*)&vertex;
		const size_t DIFF2 = (const char*)&ATT2 - (const char*)&vertex;

		vertex.get<Demo_Attribute2>().z = 10.f;
	}
}

namespace glw
{
	/*
		Stores vertices in the shader storage buffer object(SSBO).
	*/
	template<typename VertexT>
	class	VertexBuffer : public Buffer_of<VertexT>
	{
	private: // subtypes
		using	MyBase = Buffer_of<VertexT>;

	public: // functions
		using	MyBase::contain_action;

	public: // lifecycle
		CLASS_CTOR	VertexBuffer(const Buffer::Usage USAGE = Buffer::STATIC_DRAW)
			: MyBase(Buffer::TypeID::SHADER_STORAGE_BUFFER, USAGE)
		{

		}
	};


	/*
		Stores the vertex indices of the primitives.
	*/
	class	VertexTopology : public Buffer_of<uint32_t>
	{
	public: // subtypes
		enum	Primitive : uint16_t
		{
			POINTS						= GL_POINTS, 

			LINES						= GL_LINES, 
			LINE_LOOP					= GL_LINE_LOOP, 
			LINE_STRIP					= GL_LINE_STRIP, 	
			LINE_STRIP_ADJACENCY		= GL_LINE_STRIP_ADJACENCY, 
			LINES_ADJACENCY				= GL_LINES_ADJACENCY, 

			TRIANGLES					= GL_TRIANGLES,
			TRIANGLE_STRIP				= GL_TRIANGLE_STRIP, 
			TRIANGLE_FAN				= GL_TRIANGLE_FAN, 
			TRIANGLES_ADJACENCY			= GL_TRIANGLES_ADJACENCY, 
			TRIANGLE_STRIP_ADJACENCY	= GL_TRIANGLE_STRIP_ADJACENCY
		};

	public: // constants
		static const GLuint INDEX_TYPE = GL_UNSIGNED_INT;

	public: // data
		dpl::ReadOnly<Primitive, VertexTopology> primitive;

	public: // lifecycle
		CLASS_CTOR	VertexTopology(	const Primitive		PRIMITIVE,
									const Buffer::Usage USAGE = Buffer::STATIC_DRAW)
			: Buffer_of(Buffer::TypeID::ELEMENT_ARRAY_BUFFER, USAGE)
			, primitive(PRIMITIVE)
		{

		}
	};
}