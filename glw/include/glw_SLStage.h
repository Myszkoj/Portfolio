#pragma once


#include "glw_SLBlueprint.h"


namespace glw
{
	/*
		Type of the shader must be embaded in its name in the same way as file extensions.
		GEOMETRY_SHADER - .gsh
		VERTEX_SHADER	- .vsh
		FRAGMENT_SHADER - .fsh
	*/
	class SLStage : public dpl::Compendium<SLCode> // Observes only non-uniform ExternalStreams
	{
	private: // subtypes
		using ExternalStreams = Compendium<SLCode>;

	public: // subtypes
		using Type			= SLBlueprint::Type;
		using Name			= std::string;
		using SourceCode	= std::string;
		using Externals		= std::unordered_map<Name, SourceCode>;
		using Uniforms		= std::vector<SLVariable>; // All variables are unpacked(there are no variables with struct type).

	public: // data
		dpl::ReadOnly<Type,		SLStage>	type;
		dpl::ReadOnly<GLuint,	SLStage>	ID;

	protected: // lifecycle
		CLASS_CTOR					SLStage(					const SLBlueprint&								BLUEPRINT);

		CLASS_CTOR					SLStage(					SLStage&&										other) noexcept;

		CLASS_DTOR					~SLStage();

		SLStage&					operator=(					dpl::Swap<SLStage>								other) noexcept;

	private: // lifecycle
		CLASS_CTOR					SLStage(					const SLStage&									OTHER) = delete;

		SLStage&					operator=(					SLStage&&										other) noexcept = delete;

		SLStage&					operator=(					const SLStage&									OTHER) = delete;

	public: // functions
		inline void					for_each_external_transfer(	std::function<void(const ExternalSLTransfer&)>	function) const
		{
			for_each_subject([&](const SLCode& CODE)
			{
				function(static_cast<const ExternalSLTransfer&>(CODE));
			});
		}	

	public: // interface
		virtual const std::string&	get_name() const = 0;

	private: // functions
		std::string					get_shader_code(			const SourceCode&								source) const;

		void						compile(					const SourceCode&								SOURCE,
																const std::string&								LABEL);

		void						generate_and_compile(		const Type										TYPE,
																const std::string&								SOURCE_CODE,
																const std::string&								LABEL);

		void						release_glObj();

		/*
		void						expand(						Parser&											parser,
																SourceCode&										expandedCode,
																Externals&										includedFiles);

		glsl::Struct*				register_struct_type(		const Type::Name&							typeName,
																Type::Map&								customTypes) const;

		std::shared_ptr<const glsl::Type>	find_uniform_type(	const glsl::Type::Name&							typeName,
																const glsl::Type::Map&							customTypes) const;

		void						unwrap_uniform(				const std::string&								prefix,
																const glsl::Variable&							uniform);

		glsl::Variable				parse_variable(				const std::shared_ptr<const glsl::Type>&		variableType,
																Parser&											parser);

		void						parse_struct(				Parser&											parser,
																glsl::Type::Map&								customTypes);

		void						parse_code(					Parser&											parser, 
																glsl::Type::Map&								customTypes);
		*/
	};

		
	template<SLBlueprint::Type TYPE>
	class StageType : public dpl::Resource<StageType<TYPE>>
					, public SLStage
	{
	private: // subtype
		using	MyResourceBase	= dpl::Resource<StageType<TYPE>>;

	protected: // subtypes
		using	Binding	= typename MyResourceBase::Binding;

	public: //lifecycle
		CLASS_CTOR					StageType(		const dpl::Ownership&	OWNERSHIP,
													const Binding&			BINDING,
													const SLBlueprint&		BLUEPRINT)
			: MyResourceBase(OWNERSHIP, BINDING)
			, SLStage(BLUEPRINT)
		{
			if(this->type() != TYPE)
				throw dpl::GeneralException(this, __LINE__, "Invalid shader type: " + BLUEPRINT.name());
		}

		CLASS_CTOR					StageType(		const dpl::Ownership&	OWNERSHIP,
													StageType&&				other) noexcept
			: MyResourceBase(OWNERSHIP, std::move(other))
			, SLStage(std::move(other))
		{

		}

		inline StageType&			operator=(		dpl::Swap<StageType>	other) noexcept
		{
			MyResourceBase::operator=(dpl::Swap(*other));
			SLStage::operator=(dpl::Swap<SLStage>(*other));
			return *this;
		}

	private: // lifecycle
		CLASS_CTOR					StageType(		StageType&&				other) noexcept = delete;

		CLASS_CTOR					StageType(		const StageType&		other) = delete;

		StageType&					operator=(		StageType&&				other) noexcept = delete;

		StageType&					operator=(		const StageType&		other) = delete;

	public: // implementation
		virtual const std::string&	get_name() const final override
		{
			return MyResourceBase::get_label();
		}
	};


	using VertexStage	= StageType<SLBlueprint::eVERTEX_SHADER>;
	using FragmentStage = StageType<SLBlueprint::eFRAGMENT_SHADER>;
	using GeometryStage = StageType<SLBlueprint::eGEOMETRY_SHADER>;
}