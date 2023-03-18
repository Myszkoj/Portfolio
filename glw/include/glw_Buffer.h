#pragma once


#include <array>
#include <any>
#include <dpl_DataTransfer.h>
#include "glw_Utilities.h"

#pragma warning( disable : 26812)


namespace glw
{
	template<typename DataT>
	struct	DataQuery
	{
		std::type_index memberType;
	};



	class	Buffer
	{
	public: // subtypes
		enum	TypeID
		{
			ARRAY_BUFFER,
			ATOMIC_COUNTER_BUFFER,
			COPY_READ_BUFFER,
			COPY_WRITE_BUFFER,
			DISPATCH_INDIRECT_BUFFER,
			DRAW_INDIRECT_BUFFER,
			ELEMENT_ARRAY_BUFFER,
			PIXEL_PACK_BUFFER,
			PIXEL_UNPACK_BUFFER,
			QUERY_BUFFER,
			SHADER_STORAGE_BUFFER,
			TEXTURE_BUFFER, // Deprecated
			TRANSFORM_FEEDBACK_BUFFER,
			UNIFORM_BUFFER,

			NUM_BUFFER_TYPES
		};

		enum	Usage
		{
			STREAM_DRAW		= GL_STREAM_DRAW,
			STREAM_READ		= GL_STREAM_READ,
			STREAM_COPY		= GL_STREAM_COPY,

			STATIC_DRAW		= GL_STATIC_DRAW,
			STATIC_READ		= GL_STATIC_READ,
			STATIC_COPY		= GL_STATIC_COPY,

			DYNAMIC_DRAW	= GL_DYNAMIC_DRAW,
			DYNAMIC_READ	= GL_DYNAMIC_READ,
			DYNAMIC_COPY	= GL_DYNAMIC_COPY
		};

		struct	Binding
		{
			GLenum type		= GL_NONE;
			GLuint bufferID	= GL_NONE;
		};

		using	Bindings	= std::array<Binding, NUM_BUFFER_TYPES>;
		using	Action		= std::function<void()>;

	public: // data
		dpl::ReadOnly<TypeID,	Buffer>	typeID;
		dpl::ReadOnly<Usage,	Buffer>	usage;

	private: // data
		dpl::ReadOnly<GLuint,	Buffer>	ID;
		static Bindings					bindings;

	protected: // lifecycle
		CLASS_CTOR				Buffer(			const TypeID		TYPE_ID,
												const Usage			USAGE)
			: typeID(TYPE_ID)
			, usage(USAGE)
			, ID(0)
		{
			glGenBuffers(1, &*ID);
			validate_opengl(__FILE__, __LINE__, "Fail to generate buffer.");
		}

		CLASS_CTOR				Buffer(			const Buffer&		OTHER) = delete;
				
		CLASS_CTOR				Buffer(			Buffer&&			other) noexcept
			: typeID(other.typeID)
			, usage(other.usage)
			, ID(other.ID)
		{
			other.ID = 0;
		}

		CLASS_DTOR				~Buffer()
		{
			dpl::no_except([&]()
			{
				if(ID != 0u)
				{
					auto& binding = bindings[type()];
					if(binding.bufferID == ID())
					{
						glBindBuffer(binding.type, GL_NONE);
						binding.bufferID = GL_NONE;
					}
					glDeleteBuffers(1, &*ID);
					ID = 0u;
				}
			});
		}

		Buffer&					operator=(		const Buffer&		OTHER) = delete;

		Buffer&					operator=(		Buffer&&			other) noexcept
		{
			return operator=(dpl::Swap<Buffer>(other));
		}

		Buffer&					operator=(		dpl::Swap<Buffer>	other) noexcept
		{
			typeID.swap(other->typeID);
			usage.swap(other->usage);
			ID.swap(other->ID);
			return *this;
		}

	public: // functions
		inline GLenum			type() const
		{
			return bindings[typeID()].type;
		}

		inline void				bind() const
		{
			bind_buffer(typeID(), ID);
			on_binded();
		}

		inline void				unbind() const
		{
			bind_default(typeID());
		}

		void					enclose_action(	const Action&		ACTION) const
		{
			bind();
			ACTION();
			validate_opengl(__FILE__, __LINE__, "Action failed.");
			unbind();
		}

		static inline void		bind_default(	const TypeID		TYPE_ID)
		{
			bind_buffer(TYPE_ID, GL_NONE);
		}

		static void				invalidate_bindings()
		{
			for(auto& binding : bindings)
			{
				binding.bufferID = GL_NONE;
			}
		}

	private: // functions
		static void				bind_buffer(	const TypeID		TYPE_ID,
												const GLuint		BUFFER_ID)
		{
			auto& binding = bindings[TYPE_ID];
			if(binding.bufferID == BUFFER_ID) return;	
			glBindBuffer(binding.type, BUFFER_ID);
			validate_opengl(__FILE__, __LINE__, "Fail to bind buffer.");
			binding.bufferID = BUFFER_ID;		
		}

	private: // interface
		virtual void			on_binded() const{}
	};



	template<typename DataT>
	class	Buffer_of	: public Buffer
						, public dpl::BufferTransfer<DataT>
	{
	private: // subtypes
		using MyTransferBase = dpl::BufferTransfer<DataT>;

	public: // lifecycle
		CLASS_CTOR				Buffer_of(			const TypeID					TYPE_ID,
													const Usage						USAGE)
			: Buffer(TYPE_ID, USAGE)
		{

		}

		CLASS_CTOR				Buffer_of(			const Buffer_of&				OTHER) = delete;
				
		CLASS_CTOR				Buffer_of(			Buffer_of&&						other) noexcept
			: Buffer(std::move(other))
			, MyTransferBase(std::move(other))
		{

		}

		Buffer_of&				operator=(			const Buffer_of&				OTHER) = delete;

		Buffer_of&				operator=(			Buffer_of&&						other) noexcept
		{
			return operator=(dpl::Swap<Buffer_of>(other));
		}

		Buffer_of&				operator=(			dpl::Swap<Buffer_of>			other) noexcept
		{
			Buffer::operator=(dpl::Swap<Buffer>(*other));
			MyTransferBase::operator=(dpl::Swap<MyTransferBase>(*other));
			return *this;
		}

	public: // functions
		inline size_t			bits() const
		{
			return sizeof(DataT) * (size_t)MyTransferBase::size();
		}

	private: // interface
		virtual void			on_binded() const final override
		{
			MyTransferBase::update();
		}

		virtual void			on_resized() const final override
		{
			bind();
			glBufferData(type(), bits(), nullptr, usage());
			validate_opengl(__FILE__, __LINE__, "Fail to rebuild buffer.");
		}

		virtual void			on_flush_array(		const dpl::IndexRange<uint32_t>	PACK_RANGE,
													const DataT*					PACK_DATA) const final override
		{
			bind();
			glBufferSubData(type(), PACK_RANGE.begin() * sizeof(DataT), PACK_RANGE.size() * sizeof(DataT), PACK_DATA);
			validate_opengl(__FILE__, __LINE__, "Fail to transfer data to the GPU");
		}

		virtual void			on_restore_array(	const dpl::IndexRange<uint32_t>	PACK_RANGE,
													DataT*							packData) const final override
		{
			bind();
			glGetBufferSubData(type(), PACK_RANGE.begin() * sizeof(DataT), PACK_RANGE.size() * sizeof(DataT), packData);
			validate_opengl(__FILE__, __LINE__, "Fail to transfer data back to the CPU memory.");
		}
	};
}