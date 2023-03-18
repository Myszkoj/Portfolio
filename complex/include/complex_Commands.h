#pragma once


#include <unordered_map>
#include <dpl_Command.h>
#include "complex_Object.h"


namespace complex
{
	template<typename ObjT>
	using	CreateObject		= typename ObjT::CreateObject;

	template<typename ObjT>
	using	CreateObjects		= typename ObjT::CreateObjects;

	template<typename ObjT>
	using	InvalidateObject	= typename ObjT::InvalidateObject;

	template<typename ObjT>
	using	InvalidateObjects	= typename ObjT::InvalidateObjects;

	/*
		Executes batched commands that create any object type.
	*/
	class	CreateObjectComposition : public dpl::Command
	{
	private: // data
		std::unordered_map<std::type_index, std::unique_ptr<dpl::Command>> map;

	public: // lifecycle
		CLASS_CTOR				CreateObjectComposition() = default;

	public: // functions
		template<typename ObjT>
		CreateObjects<ObjT>&	command_of()
		{
			auto& command = map[typeid(CreateObjects<ObjT>)];
			if(!command) command = std::make_unique<CreateObjects<ObjT>>();
			return static_cast<CreateObjects<ObjT>&>(*command.get());
		}

		template<typename ObjT>
		inline void				reserve(	const uint32_t	OBJECTS_TO_ADD)
		{
			CreateObjectTypes::command_of<ObjT>().reserve(OBJECTS_TO_ADD);
		}
		
	private: // functions
		template<typename ObjT>
		inline dpl::Command&	raw_command()
		{
			return CreateObjectTypes::command_of<ObjT>();
		}

	private: // implementation
		virtual bool			valid() const final override
		{
			for(const auto& IT : map)
				if(IT.second->valid())
					return true;

			return false;
		}

		virtual void			execute() final override
		{
			for(auto& it : map) it.second->execute();
		}

		virtual void			unexecute() final override
		{
			for(auto& it : map) it.second->unexecute();
		}
	};

	/*
		Executes batched commands that invalidate any object type.
	*/
	class	InvalidateObjectComposition : public dpl::Command
	{
	private: // data
		std::unordered_map<std::type_index, std::unique_ptr<dpl::Command>> map;

	public: // lifecycle
		CLASS_CTOR					InvalidateObjectComposition() = default;

	public: // functions
		template<typename ObjT>
		InvalidateObjects<ObjT>&	command_of()
		{
			auto& command = map[typeid(InvalidateObjects<ObjT>)];
			if(!command) command = std::make_unique<InvalidateObjects<ObjT>>();
			return static_cast<InvalidateObjects<ObjT>&>(*command.get());
		}

		template<typename ObjT>
		inline void					reserve(	const uint32_t	OBJECTS_TO_INVALIDATE)
		{
			InvalidateObjectTypes::command_of<ObjT>().reserve(OBJECTS_TO_INVALIDATE);
		}

	private: // functions
		template<typename ObjT>
		inline dpl::Command&		raw_command()
		{
			return InvalidateObjectTypes::command_of<ObjT>();
		}

	private: // implementation
		virtual bool				valid() const final override
		{
			for(const auto& IT : map)
				if(IT.second->valid())
					return true;

			return false;
		}

		virtual void				execute() final override
		{
			for(auto& it : map) it.second->execute();
		}

		virtual void				unexecute() final override
		{
			for(auto& it : map) it.second->unexecute();
		}
	};

	template<typename ParentT, typename... ChildTs>
	class	InvalidateChildren : public dpl::Command
	{
	private: // data
		InvalidateObjectTypes command;

	public: // lifecycle
		CLASS_CTOR		InvalidateChildren(	const ParentT& PARENT)
		{
			(command.command_of<ChildTs>().invalidate_children_of(PARENT), ...);
		}

	private: // implementation
		virtual bool	valid() const final override
		{
			return command.valid();
		}

		virtual void	execute() final override
		{
			command.execute();
		}

		virtual void	unexecute() final override
		{
			command.unexecute();
		}
	};
}