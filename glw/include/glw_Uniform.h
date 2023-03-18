#pragma once


#include "glw_SLProgram.h"
#include "glw_TextureBuffer.h"


namespace glw
{
	template<typename glslT>
	class Uniform	: public ExternalSLTransfer
	{
	public: // subtypes
		using DataT	= typename glslT::DataT;

	public: // lifecycle
		CLASS_CTOR			Uniform(		SLType&					type,
											const std::string&		NAME,
											const uint32_t			NUM_ELEMENTS = 1)
			: ExternalSLTransfer(eUNIFORM, UNKNOWN_LOCATION, type, NAME, NUM_ELEMENTS)
		{
			auto* uniformType = dynamic_cast<const glslT*>(&type);
			if(uniformType == nullptr)
				throw dpl::GeneralException(this, __LINE__, "Invalid uniform: " + NAME + " Type does not match type specified by the user: " + type.name());
		}

		CLASS_CTOR			Uniform(		const std::string&		NAME,
											const uint32_t			NUM_ELEMENTS = 1)
			: Uniform(*glw::get_default_type(glslT::NAME), NAME, NUM_ELEMENTS)
		{

		}

		inline Uniform&		operator=(		dpl::Swap<Uniform>		other)
		{
			ExternalSLTransfer::operator=(dpl::Swap<ExternalSLTransfer>(*other));
			return *this;
		}

	public: // functions
		/*
			Update single element in the uniform array.
		*/
		inline void			update_element(	const DataT&			VALUE,
											const uint32_t			OFFSET) const
		{ 
			validate_location();
			if(OFFSET >= this->numElements())
				throw dpl::GeneralException(this, __LINE__, "Invalid offset(" + std::to_string(OFFSET) + "): " + this->string());

			static_cast<const glslT&>(this->type()).update_uniform(location(), VALUE, OFFSET);
		}

		/*
			Updates uniform when number of elements is equal 1.
		*/
		inline void			update(			const DataT&			DATA) const
		{
			update_element(DATA, 0);
		}

		/*
			Updates whole array of uniform elements.
		*/
		inline void			update_array(	const DataT*			ARRAY) const
		{ 
			validate_location();
			if(!ARRAY) throw dpl::GeneralException(this, __LINE__, "Array is missing: " + this->string());
			static_cast<const glslT*>(this->type().get())->update_uniform_array(location(), ARRAY, this->numElements());
		}

	private: // functions
		inline void			validate_location() const
		{
			if(location == UNKNOWN_LOCATION)
				throw dpl::GeneralException(this, __LINE__, "Uniform location is invalid.");
		}
	};


	/*
		Handles access to the gl::Data through samplers.
		Declares 'PREFIX'_OFFSET and 'PREFIX'_SAMPLER uniforms, 
		where 'PREFIX' is chosen by the user during DataAccess construction.
	*/
	template<typename glslT> 
	class UniformPartition // Legacy name: Revac::glsl::DataAccess
	{
	private: // subtypes
		using SamplerT	= typename SamplerBufferSelector<typename glslT::ComponentT>::Type;
		using DataT		= typename glslT::DataT;
		using MyData	= Partition<DataT>;

	public: // constants
		static const uint32_t DEFAULT_SAMPLER_UNIT = 0;

	private: // data
		mutable Uniform<SamplerT>	uSAMPLER;
		mutable Uniform<Int>		uSAMPLER_OFFSET;
		uint32_t					m_samplerUnit;

	public: // lifecycle
		CLASS_CTOR					UniformPartition(	const std::string&			PREFIX,
														const uint32_t				SAMPLER_UNIT = DEFAULT_SAMPLER_UNIT)
			: uSAMPLER(PREFIX + "_SAMPLER")
			, uSAMPLER_OFFSET(PREFIX + "_OFFSET")
			, m_samplerUnit(SAMPLER_UNIT)
		{

		}

		UniformPartition&			operator=(			dpl::Swap<UniformPartition>	other)
		{
			uSAMPLER		= dpl::Swap(other->uSAMPLER);
			uSAMPLER_OFFSET = dpl::Swap(other->uSAMPLER_OFFSET);
			std::swap(m_samplerUnit, other->m_samplerUnit);
			return *this;
		}

	public: // functions
		inline const std::string&	name() const
		{
			return uSAMPLER.name();
		}

		inline void					design_shader(		SLBlueprint&			blueprint) const
		{
			blueprint.bind_stream(uSAMPLER);
			blueprint.bind_stream(uSAMPLER_OFFSET);
		}

		inline void					bind_program(		SLProgram&				program)
		{
			uSAMPLER.bind_program(program);
			uSAMPLER_OFFSET.bind_program(program);
		}

		inline void					update_program(		const Partition<DataT>&	PARTITION) const
		{
			if(PARTITION.has_buffer())
			{
				const Buffer_of<DataT>& BUFFER = PARTITION.get_buffer();
				if(BUFFER.type() == Buffer::Type::eTEXTURE_BUFFER)
				{
					const TextureBuffer<glslT>&	TEXTURE_BUFFER = static_cast<const TextureBuffer<glslT>&>(BUFFER);
												TEXTURE_BUFFER.bind_for_reading(m_samplerUnit);

					uSAMPLER.update(m_samplerUnit);
					uSAMPLER_OFFSET.update(PARTITION.offset());
				}
			}
		}
	};


	/*
		Stores value and uniform used to transfer that value to the shader.
		Legacy name: Revac::glsl::Programmable
	*/
	template<typename glslT>
	class UniformValueStorage
	{
	public: // subtypes
		using DataT = typename glslT::DataT;

	private: // data
		mutable Uniform<glslT>	uVALUE;
		DataT					m_value;

	public: // lifecycle
		CLASS_CTOR					UniformValueStorage(	const std::string&		NAME,
															const DataT&			VALUE)
			: uVALUE(NAME, 1)
			, m_value(VALUE)
		{

		}

		inline UniformValueStorage&	operator=(				dpl::Swap<UniformValueStorage>	other)
		{
			uVALUE = dpl::Swap(other->uVALUE);
			std::swap(m_value,other->m_value);
			return *this;
		}

	public: // functions
		inline void					set(					const DataT&			NEW_VALUE)
		{
			m_value = NEW_VALUE;
		}

		inline const DataT&			get() const
		{
			return m_value;
		}

		inline const std::string&	name() const
		{
			return uVALUE.name();
		}

	public: // interface
		inline void					design_shader(		SLBlueprint&			blueprint) const
		{
			blueprint.bind_transfer(uVALUE);
		}

		/*
			Attaches programmable to given shader program.
		*/
		inline void					bind_program(		SLProgram&				program)
		{
			uVALUE.bind_program(program);
		}

		/*
			This function can only be called when program is bined.
			Updates state of the uniform.
		*/
		inline void					update_program() const
		{
			uVALUE.update(value());
		}
	};


	/*
		Stores buffer partition and sampler uniform used to access data in the shader.
		Legacy name: Revac::glsl::ProgrammableData
	*/
	template<typename glslT> 
	class UniformPartitionStorage	: public UniformPartition<glslT>
									, public Partition<typename glslT::DataT>
	{
	private: // subtypes
		using DataType			= typename glslT::DataT;
		using MyUniformBase		= UniformPartition<glslT>;
		using MyPartitionBase	= Partition<DataType>;

	public: // constants
		static const uint32_t DEFAULT_SAMPLER_UNIT = UniformPartition<glslT>::DEFAULT_SAMPLER_UNIT;

	public: // lifecycle
		CLASS_CTOR						UniformPartitionStorage(	Buffer_of<DataType>&				buffer,
																	const std::string&					PREFIX,
																	const bool							bKEEP_FLUSHED_DATA	= true,
																	const uint32_t						SAMPLER_UNIT		= DEFAULT_SAMPLER_UNIT)
			: MyPartitionBase(buffer, bKEEP_FLUSHED_DATA)
			, MyUniformBase(PREFIX, SAMPLER_UNIT)
		{

		}

		inline UniformPartitionStorage&	operator=(					dpl::Swap<UniformPartitionStorage>	other)
		{
			MyUniformBase::operator=(dpl::Swap<MyUniformBase>(*other));
			MyPartitionBase::operator=(dpl::Swap<MyPartitionBase>(*other));
			return *this;
		}

	public: // Programmable extension
		inline void						design_shader(				SLBlueprint&						blueprint) const final override
		{
			MySampler::design_shader(blueprint);
			blueprint.bind_stream(uOFFSET);
		}

		inline void						bind_program(				SLProgram&							program) final override
		{
			MySampler::bind_program(program);
			uOFFSET.bind_program(program);
		}

		inline void						update_program() const final override
		{
			MyUniformBase::update_program(*this);
		}
	};
}