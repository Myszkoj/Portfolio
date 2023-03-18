#pragma once


#include "glw_Types.h"
#include "glw_SLCode.h"


namespace glw
{
	class SLVariable	: public dpl::Observer<SLType>
						, public SLCode // NAME
	{
	public: // types
		using Table	= std::vector<SLVariable>;

	public: // data
		dpl::ReadOnly<uint32_t,	SLVariable> numElements; // 0 => [], 1 => no array, N>1 => [N]

	public: // lifecycle
		CLASS_CTOR					SLVariable(			SLType&					type,
														const std::string&		NAME,
														const uint32_t			NUM_ELEMENTS = 1);

		CLASS_CTOR					SLVariable(			const SLVariable&		OTHER) = delete;

		CLASS_CTOR					SLVariable(			SLVariable&&			other) noexcept = default;

		CLASS_CTOR					SLVariable(			std::istream&			binary);

		inline const SLVariable&	operator>>(			std::ostream&			binary) const
		{
			SLCode::operator>>(binary);
			export_string(binary, type().name());
			dpl::export_t(binary, numElements());
			return *this;
		}

		inline SLVariable&			operator=(			dpl::Swap<SLVariable>	other)
		{
			SLCode::operator=(dpl::Swap<SLCode>(*other));
			Observer::operator=(dpl::Swap<dpl::Observer<SLType>>(*other));
			numElements.swap(other->numElements);
			return *this;
		}

		inline SLVariable&			operator=(			const SLVariable&		OTHER) = delete;

		inline SLVariable&			operator=(			SLVariable&&			other) noexcept = default;

	public: // functions
		inline bool					is_valid() const
		{
			return Observer::has_subject();
		}

		inline const SLType&		type() const
		{
			return *Observer::get_subject();
		}

		inline const std::string&	name() const
		{
			return SLCode::string();
		}

	public: // interface
		virtual void				expand_source_code(	std::string&			sourceCode) const override;
	};
}