#include "..//include/glw_SLTransfers.h"
#include "..//include/glw_SLProgram.h"


namespace glw
{
	void	InternalSLTransfer::expand_source_code(	std::string&	sourceCode) const
	{
		switch(interpolation)
		{
		case Interpolation::eFLAT:
			sourceCode += (mode == Mode::eIN)	? "flat in "
												: "flat out ";
			break;
		case Interpolation::eNOPERSPECTIVE:
			sourceCode += (mode == Mode::eIN)	? "noperspective in "
												: "noperspective out ";
			break;
		case Interpolation::eSMOOTH:
			sourceCode += (mode == Mode::eIN)	? "smooth in "
												: "smooth out ";
			break;
		default:
			sourceCode += (mode == Mode::eIN)	? "in "
												: "out ";
		}

		SLVariable::expand_source_code(sourceCode);
	}

	void	ExternalSLTransfer::bind_program(		SLProgram&		program) const
	{
		switch(qualifier())
		{
		case Qualifier::eATTRIBUTE:
			if(location() == UNKNOWN_LOCATION)
			{
				this->location = program.get_attribute_location(this->string());

				//if(location() == -1)
				//	throw Crash(this, __LINE__, "Could not find attribute with given name: " + name());
			}
			else
			{
				program.set_attribute_location(this->string(), location());
			}
			break;

		case Qualifier::eFRAG_DATA:
			if(location() == UNKNOWN_LOCATION)
			{
				this->location = program.get_fragData_location(this->string());
			}
			else
			{
				program.set_fragData_location(this->string(), this->location());
			}
			break;

		case Qualifier::eUNIFORM:
			this->location = program.get_uniform_location(this->string());

			//if(location() == -1)
			//	throw Crash(this, __LINE__, "Could not find uniform with given name: " + name());

			break;

		default:
			throw dpl::GeneralException(this, __LINE__, "Unknown qualifier: " + std::to_string(qualifier()));
		}
	}

	void	ExternalSLTransfer::expand_source_code(	std::string&	sourceCode) const
	{
		if(qualifier == eUNIFORM)
		{
			sourceCode += "uniform ";
		}
		else
		{
			sourceCode += (mode == Mode::eIN)	? "in "
												: "out ";
		}

		SLVariable::expand_source_code(sourceCode);
	}
}