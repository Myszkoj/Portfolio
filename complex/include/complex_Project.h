#pragma once


#include <optional>
#include <dpl_Singleton.h>
#include <dpl_Command.h>
#include <dpl_Logger.h>
#include "complex_Utilities.h"


namespace complex
{
	class Application;


	class Project	: public dpl::Singleton<Project>
					, private dpl::CommandInvoker
	{
	public: // relations
		friend	Application;

	public: // subtypes
		enum	Flags
		{
			OPEN,
			LOCKED, //?
			UNSAVED
		};

		using	OnSetPath	= std::function<bool(const std::string&)>;

	public: // data
		static dpl::ReadOnly<std::string,	Project>	ext;
		dpl::ReadOnly<dpl::Mask32<Flags>,	Project>	flags;
		dpl::ReadOnly<std::string,			Project>	name;
		dpl::ReadOnly<std::string,			Project>	folder;

	public: // command functions
		using	CommandInvoker::invoke;
		using	CommandInvoker::undo;
		using	CommandInvoker::redo;
		using	CommandInvoker::clear;

	public: // lifecycle
		CLASS_CTOR				Project()
		{
			flags->set_at(UNSAVED, true);
		}

	public: // functions
		static bool				validate_extension()
		{
			if(ext().empty()) throw dpl::GeneralException(__FILE__, __LINE__, "Extension was not set.");
		}

		static void				set_extension(		const std::string_view&	DOT_EXT)
		{
			if(!ext().empty())		throw dpl::GeneralException(__FILE__, __LINE__, "Extension already set.");
			if(DOT_EXT.size() < 2)	throw dpl::GeneralException(__FILE__, __LINE__, "Extension size too small.");
			if(DOT_EXT[0] != '.')	throw dpl::GeneralException(__FILE__, __LINE__, "Extension must start with '.'");
			ext = DOT_EXT;
		}

		inline std::string		get_fullPath() const
		{
			return folder() + name() + ext();
		}

		inline void				set_unsaved()
		{
			flags->set_at(UNSAVED, true);
		}

	private: // functions
		inline void				set_saved()
		{
			flags->set_at(UNSAVED, false);
		}

		bool					set_path(			const std::string&		FILE_PATH,
													dpl::Logger&			logger,
													const OnSetPath&		FUNCTION);

		void					close();
	};
}