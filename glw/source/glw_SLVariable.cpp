#include "..//include/glw_SLVariable.h"


namespace glw
{
//=====> SLVariable public: // lifecycle
	CLASS_CTOR	SLVariable::SLVariable(			SLType&					type,
												const std::string&		NAME,
												const uint32_t			NUM_ELEMENTS)
		: SLCode(NAME)
		, numElements(NUM_ELEMENTS)
	{
		if(NUM_ELEMENTS == 0)
			throw dpl::GeneralException(this, __LINE__, "Variable(array) cannot have 0 elements.");

		observe(type);
	}

	CLASS_CTOR	SLVariable::SLVariable(			std::istream&			binary)
		: SLCode(binary)
	{
		if(auto* type = get_default_type(import_string(binary)))
		{
			observe(*type);
			dpl::import_t(binary, *numElements);
		}
		else
		{
			throw dpl::GeneralException(this, __LINE__, "Fail to import: variable type is unknown.");
		}
	}

//=====> SLVariable public: // functions
	void		SLVariable::expand_source_code(	std::string&			sourceCode) const
	{
		switch(numElements())
		{
		case 0:		sourceCode += type().name() + " " + this->string() + "[];"; break; // Unspecified array.
		case 1:		sourceCode += type().name() + " " + this->string() + ";"; break;
		default:	sourceCode += type().name() + " " + this->string() + "[" + std::to_string(numElements) + "];"; break;
		}
	}
}