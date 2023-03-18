#pragma once


#include "glw_Utilities.h"


namespace glw
{
	class SLCode : public dpl::Subject<SLCode>
	{
	public: // data
		dpl::ReadOnly<std::string, SLCode> string;

	public: // lifecycle
		CLASS_CTOR				SLCode(				const std::string&	STRING)
			: string(STRING)
		{
			if(string().size() == 0)
				throw dpl::GeneralException(this, __LINE__, "Code string is missing.");
		}

		CLASS_CTOR				SLCode(				std::istream&		binary)
		{
			import_string(binary, *string);
		}

		CLASS_CTOR				SLCode(				const SLCode&		OTHER)
			: string(OTHER.string)
		{

		}

		CLASS_CTOR				SLCode(				SLCode&&			other) noexcept = default;

		CLASS_DTOR virtual		~SLCode() = default;

	public: // operators
		inline SLCode&			operator=(			const SLCode&		OTHER)
		{
			string = OTHER.string;
			return *this;
		}

		inline SLCode&			operator=(			SLCode&&			OTHER) noexcept = default;

		inline const SLCode&	operator>>(			std::ostream&		binary) const
		{
			export_string(binary, string());
			return *this;
		}

		inline SLCode&			operator=(			dpl::Swap<SLCode>	other)
		{
			Subject::operator=(dpl::Swap(*other));
			std::swap(*string, *other->string);
			return *this;
		}

	public: // interface
		virtual void			expand_source_code(	std::string&		sourceCode) const
		{
			sourceCode += string;
		}
	};
}