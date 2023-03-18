#include "..//include/glw_SLBlueprint.h"


namespace glw
{
	void						SLBlueprint::Region::expand_source_code(std::string&			sourceCode) const
	{
		sourceCode += string();
		if(bClosed)
		{
			sourceCode += "\n{";
		}
			
		Compendium::for_each_subject([&](const SLCode& CODE)
		{
			sourceCode += "\n";
			CODE.expand_source_code(sourceCode);
		});

		if(bClosed)
		{
			sourceCode += "\n}";
		}
	}

//=====> SLBlueprint public: // constants
	const char*const			SLBlueprint::GLOBAL_REGION	= "GLOBAL";
	const char*const			SLBlueprint::MAIN_REGION		= "MAIN";

//=====> SLBlueprint public: // lifecycle
	CLASS_CTOR					SLBlueprint::SLBlueprint(				const Type				TYPE,
																		const std::string&		NAME,
																		const Version			VERSION)
		: type(TYPE)
		, name(NAME)
	{
		m_regions.try_emplace(GLOBAL_REGION, "#version " + std::to_string(VERSION), false);
		m_regions.try_emplace(MAIN_REGION, "\nvoid main()", true);
	}

//=====> SLBlueprint public: // functions
	bool						SLBlueprint::create_region(				const std::string&		PARENT_REGION,
																		const std::string&		NAME,
																		const std::string&		OPENING,
																		const bool				bCLOSED)
	{
		if(auto* parent = find_region(PARENT_REGION))
		{
			// std::piecewise_construct
			auto result = m_regions.try_emplace(NAME, OPENING, bCLOSED);
			if(result.second)
			{
				return parent->add_subject(result.first->second);
			}
		}
			
		return false;
	}

	void						SLBlueprint::bind_entry(				const std::string&		REGION_NAME,
																		SLCode&					code)
	{
		if(auto* region = find_region(REGION_NAME))
		{
			region->add_subject(code);
		}
	}

	void						SLBlueprint::for_each_external_transfer(const ProcessStream		FUNCTION) const
	{
		for(auto& it : m_regions)
		{
			auto& region = it.second;
			region.for_each_subject([&](const SLCode& CODE)
			{
				if(const auto* TRANSFER = dynamic_cast<const ExternalSLTransfer*>(&CODE))
				{
					FUNCTION(const_cast<ExternalSLTransfer&>(*TRANSFER));
				}
			});
		}
	}

	std::string					SLBlueprint::generate_source() const
	{
		std::string sourceCode;;
		find_region(GLOBAL_REGION)->expand_source_code(sourceCode);
		find_region(MAIN_REGION)->expand_source_code(sourceCode);
		return sourceCode;
	}

	SLBlueprint::Region*		SLBlueprint::find_region(				const std::string&		REGION_NAME)
	{
		auto result = m_regions.find(REGION_NAME);
		if(result != m_regions.end())
			return &result->second;

		return nullptr;
	}

	const SLBlueprint::Region*	SLBlueprint::find_region(				const std::string&		REGION_NAME) const
	{
		auto result = m_regions.find(REGION_NAME);
		if(result != m_regions.end())
			return &result->second;

		return nullptr;
	}
}