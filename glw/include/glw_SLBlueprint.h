#pragma once


#include "glw_SLTransfers.h"


namespace glw
{
	/*
		Stores code structure of the glsl shader stages.
	*/
	class SLBlueprint
	{
	public: // subtypes
		enum	Type
		{
			eGEOMETRY_SHADER	= GL_GEOMETRY_SHADER,	/* Shader name/file must end with the .gsh extension. */
			eVERTEX_SHADER		= GL_VERTEX_SHADER,		/* Shader name/file must end with the .vsh extension. */
			eFRAGMENT_SHADER	= GL_FRAGMENT_SHADER,	/* Shader name/file must end with the .fsh extension. */
			eUNKNOWN_SHADER		= 0
		};

		class	Region	: public SLCode
						, public dpl::Compendium<SLCode>
		{
		public: // relations
			friend SLBlueprint;

		private: // data
			bool	bClosed = true;

		public: // lifecycle
			/* CTOR */		Region(				const std::string&	OPENING,
												const bool			bCLOSED)
				: SLCode(OPENING)
				, bClosed(bCLOSED)
			{

			}

		public: // functions
			virtual void	expand_source_code(	std::string&		sourceCode) const;
		};

		using	Regions			= std::unordered_map<std::string, Region>;
		using	Version			= uint32_t;
		using	ProcessStream	= std::function<void(ExternalSLTransfer&)>;

	public: // constants
		static const char*const GLOBAL_REGION;
		static const char*const MAIN_REGION;

	public: // data
		dpl::ReadOnly<Type,			SLBlueprint> type;
		dpl::ReadOnly<std::string,	SLBlueprint> name;

	private: // data
		Regions m_regions;

	public: // lifecycle	
		/* CTOR */		SLBlueprint(				const Type				TYPE,
													const std::string&		NAME,
													const Version			VERSION);

	public: // functions
		bool			create_region(				const std::string&		PARENT_REGION,
													const std::string&		NAME,
													const std::string&		OPENING,
													const bool				bCLOSED = true);

		/*
			Adds pointer to the existing entry.
		*/
		void			bind_entry(					const std::string&		REGION_NAME,
													SLCode&					code);

		inline void		bind_global_entry(			SLCode&					code)
		{
			bind_entry(GLOBAL_REGION, code);
		}

		inline void		bind_main_entry(			SLCode&					code)
		{
			bind_entry(MAIN_REGION, code);
		}

		inline void		bind_transfer(				SLTransfer&				transfer)
		{
			bind_global_entry(transfer);
		}

		void			for_each_external_transfer(	const ProcessStream		FUNCTION) const;

		std::string		generate_source() const;

	private: // functions
		Region*			find_region(				const std::string&		REGION_NAME);

		const Region*	find_region(				const std::string&		REGION_NAME) const;
	};
}