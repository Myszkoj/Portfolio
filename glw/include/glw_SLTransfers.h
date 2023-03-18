#pragma once


#include "glw_SLVariable.h"


namespace glw
{
	class SLProgram;



	class SLTransfer : public SLVariable
	{
	public: // types
		enum Mode
		{
			eIN, /* Streams data into the shader stage. */
			eOUT /* Streams data from the shader stage. */
		};

	public: // data
		dpl::ReadOnly<Mode,	SLTransfer> mode;

	public: // lifecycle
		CLASS_CTOR					SLTransfer(	const Mode				MODE,
												SLType&					type,
												const std::string&		NAME,
												const uint32_t			NUM_ELEMENTS = 1)
			: SLVariable(type, NAME, NUM_ELEMENTS)
			, mode(MODE)
		{

		}

		CLASS_CTOR					SLTransfer(	std::istream&			binary)
			: SLVariable(binary)
		{
			import_t(binary, mode);
		}

		inline const SLTransfer&	operator>>(	std::ostream&			binary) const
		{
			SLVariable::operator>>(binary);
			dpl::export_t(binary, mode());
			return *this;
		}

		inline SLTransfer&			operator=(	dpl::Swap<SLTransfer>	other)
		{
			SLVariable::operator=(dpl::Swap<SLVariable>(*other));
			mode.swap(other->mode);
			return *this;
		}
	};



	/*
		Streams data between shader stages.
	*/
	class InternalSLTransfer : public SLTransfer
	{
	public: // types
		enum Interpolation
		{
			eFLAT,
			eNOPERSPECTIVE,
			eSMOOTH
		};

	public: // data
		dpl::ReadOnly<Interpolation, InternalSLTransfer> interpolation; 

	public: // lifecycle
		CLASS_CTOR							InternalSLTransfer(	const Interpolation				INTERPOLATION,
																const Mode						MODE,
																SLType&							type,
																const std::string&				NAME,
																const uint32_t					NUM_ELEMENTS = 1)
			: SLTransfer(MODE, type, NAME, NUM_ELEMENTS)
			, interpolation(INTERPOLATION)
		{
			if(type.signature() != SLType::eSCALAR
			&& type.signature() != SLType::eVECTOR
			&& type.signature() != SLType::eMATRIX)
				throw dpl::GeneralException(this, __LINE__, "Invalid type signature. Stream must be either scalar, vector or matrix variable type.");
		}

		CLASS_CTOR							InternalSLTransfer(	std::istream&					binary)
			: SLTransfer(binary)
		{
			import_t(binary, interpolation);
		}


		inline const InternalSLTransfer&	operator>>(			std::ostream&					binary) const
		{
			SLTransfer::operator>>(binary);
			export_t(binary, interpolation);
			return *this;
		}

		inline InternalSLTransfer&			operator=(			dpl::Swap<InternalSLTransfer>	other)
		{
			SLTransfer::operator=(dpl::Swap<SLTransfer>(*other));
			interpolation.swap(other->interpolation);
			return *this;
		}

	public: // functions
		virtual void						expand_source_code(	std::string&					sourceCode) const override;
	};



	/*
		Streams client data to/from shader program.
	*/
	class ExternalSLTransfer : public SLTransfer
	{
	public: // types
		enum Qualifier
		{
			eATTRIBUTE,
			eFRAG_DATA,
			eUNIFORM
		};

	public: // constants
		static const int32_t UNKNOWN_LOCATION = -1;

	public: // data
		dpl::ReadOnly<Qualifier,		ExternalSLTransfer> qualifier;
		mutable dpl::ReadOnly<int32_t,	ExternalSLTransfer> location;

	public: // lifecycle
		CLASS_CTOR							ExternalSLTransfer(	const Qualifier					QUALIFIER,
																const int32_t					LOCATION,
																SLType&							type,
																const std::string&				NAME,
																const uint32_t					NUM_ELEMENTS = 1)
			: SLTransfer((QUALIFIER == Qualifier::eFRAG_DATA) ? Mode::eOUT : Mode::eIN, type, NAME, NUM_ELEMENTS)
			, qualifier(QUALIFIER)
			, location(LOCATION)
		{
			if(this->location() < UNKNOWN_LOCATION)
				this->location = UNKNOWN_LOCATION;
			
			if(type.signature() != SLType::eSAMPLER) return;
			switch(qualifier)
			{
			case Qualifier::eATTRIBUTE: throw dpl::GeneralException(this, __LINE__, "Sampler cannot be set as vertex attribute.");
			case Qualifier::eFRAG_DATA: throw dpl::GeneralException(this, __LINE__, "Sampler cannot be set as fragment data output.");
			default: break;
			}
		}

		CLASS_CTOR							ExternalSLTransfer(	std::istream&					binary)
			: SLTransfer(binary)
		{
			dpl::import_t(binary, qualifier);
			dpl::import_t(binary, location);
		}

		inline const ExternalSLTransfer&	operator>>(			std::ostream&					binary) const
		{
			SLTransfer::operator>>(binary);
			dpl::export_t(binary, qualifier);
			dpl::export_t(binary, location);
			return *this;
		}

		inline ExternalSLTransfer&			operator=(			dpl::Swap<ExternalSLTransfer>	other)
		{
			SLTransfer::operator=(dpl::Swap<SLTransfer>(*other));
			qualifier.swap(other->qualifier);
			location.swap(other->location);
			return *this;
		}

	public: // functions
		inline bool							operator==(			const ExternalSLTransfer&		other) const
		{ 
			return this->name() == other.name();
		}

		/*
			Binds this stream to the given shader program.
		*/
		void								bind_program(		SLProgram&						program) const;

		virtual void						expand_source_code(	std::string&					sourceCode) const override;
	};



	template<typename glslT>
	class VertexAttribute : public ExternalSLTransfer
	{
	public: // subtypes
		using DataT = typename glslT::DataT;

	public: // data
		dpl::ReadOnly<uint32_t,	VertexAttribute> divisor;

	public: // lifecycle
		CLASS_CTOR						VertexAttribute(	const std::string&			NAME,
															const int32_t				LOCATION,
															const uint32_t				DIVISIOR)
			: ExternalSLTransfer(eATTRIBUTE, LOCATION, *glw::get_default_type(glslT::NAME), NAME, 1)
			, divisor(DIVISIOR)
		{
				
		}

		CLASS_CTOR						VertexAttribute(	std::istream&				binary)
			: ExternalSLTransfer(binary)
		{
			dpl::import_t(binary, *divisor);
		}

		inline const VertexAttribute&	operator>>(			std::ostream&				binary) const
		{
			ExternalSLTransfer::operator>>(binary);
			dpl::export_t(binary, divisor());
			return *this;
		}

		inline VertexAttribute&			operator=(			dpl::Swap<VertexAttribute>	other)
		{
			ExternalSLTransfer::operator=(dpl::Swap<ExternalSLTransfer>(*other));
			divisor.swap(other->divisor);
			return *this;
		}

	public: // functions
		void							enable() const
		{
			const auto		DESC	= get_typeDescription(type().glBase);
			const uint32_t	STRIDE	= DESC.bytes * type().columns() * type().rows();

			switch(type().signature())
			{
			case SLType::Signature::eSCALAR:
			case SLType::Signature::eVECTOR:
			case SLType::Signature::eMATRIX:
				for (GLuint index = 0; index < type().rows(); ++index)
				{
					const size_t BYTE_OFFSET = DESC.bytes * index * type().columns();

					glEnableVertexAttribArray(location + index);
					glVertexAttribPointer(location + index, type().columns(), type().glBase, GL_FALSE, STRIDE, (GLvoid*)BYTE_OFFSET);
					if(divisor > 0) glVertexAttribDivisor(location + index, divisor);
				}
				break;

			default:
				throw dpl::GeneralException(this, __LINE__, type().name() + " cannot be used as an attribute.");
			}
		}
	};


	template<typename glslT>
	class FragData : public ExternalSLTransfer
	{
	public: // subtypes
		using DataT = typename glslT::DataT;

	public: // lifecycle
		CLASS_CTOR			FragData(		const GLint			LOCATION,
											const std::string&	NAME)
			: ExternalSLTransfer(eFRAG_DATA, LOCATION, *glw::get_default_type(glslT::NAME), NAME, 1)
		{
			static const GLint MAX_LOCATIONS = 16; 
			if(LOCATION >= MAX_LOCATIONS)
				throw dpl::GeneralException(this, __LINE__, "Location of the " + NAME + " must be less than " + std::to_string(MAX_LOCATIONS));
		}

		inline FragData&	operator=(		dpl::Swap<FragData>	other)
		{
			ExternalSLTransfer::operator=(dpl::Swap<ExternalSLTransfer>(*other));
			return *this;
		}

		inline bool			operator==(		const FragData&		OTHER) const
		{ 
			return this->location() == OTHER.location();
		}
	};
}

namespace std 
{
	template <>
	struct hash<glw::ExternalSLTransfer>
	{
		size_t operator()(const glw::ExternalSLTransfer& TRANSFER) const
		{
			return hash<string>()(TRANSFER.string());
		}
	};
}