#pragma once


#include "complex_Application.h"
#include "complex_Object.h"
#include "complex_Commands.h"


// declarations
namespace complex
{
	class	IObjectStorage;

	template<typename ObjT>
	class	ObjectStorage;

	template<typename ObjT, ParentTypeList PARENT_TYPES, ChildTypeList CHILD_TYPES, LevelTypesList LEVEL_TYPES>
	class	GroupSpecialization;

	template<typename ObjT>
	class	ObjectGroup;
}

// object handle
namespace complex
{
	/*
		TODO: Somehow convert to any RuntimeParent<ChildT, LEVEL>
	*/
	class ObjectHandle
	{
	public: // data
		dpl::ReadOnly<uint32_t, ObjectHandle> objectGroupID;
		dpl::ReadOnly<uint32_t, ObjectHandle> objectIndex;

	private: // lifecycle
		template<typename ObjT>
		CLASS_CTOR					ObjectHandle(Member<ObjT>& object)
			: objectGroupID(object.get_groupID())
			, objectIndex(object.get_index())
		{

		}

	public: // lifecycle
		CLASS_CTOR					ObjectHandle(std::nullptr_t)
			: objectGroupID(std::numeric_limits<uint32_t>::max())
			, objectIndex(std::numeric_limits<uint32_t>::max())
		{
			
		}

		template<typename ObjT> requires std::is_base_of_v<Member<ObjT>, ObjT>
		CLASS_CTOR					ObjectHandle(ObjT& object)
			: ObjectHandle(object)
		{

		}

		template<typename ObjT> requires std::is_base_of_v<Member<ObjT>, ObjT>
		CLASS_CTOR					ObjectHandle(ObjT* object)
			: ObjectHandle(object? *object : nullptr)
		{

		}

	private: // functions
		template<typename ObjT>
		inline ObjectGroup<ObjT>&	get_ObjectGroup()
		{
#ifdef _DEBUG
			if(objectGroupID() != ObjectGroup<ObjT>::get_typeID())
				throw std::bad_cast;
#endif // _DEBUG

			return ObjectGroup<ObjT>::ref();
		}

	public: // conversion operators
		template<typename ObjT>
		inline operator ObjT&()
		{
			return *get_ObjectGroup<ObjT>.get_object_at(objectGroupID);
		}

		template<typename ObjT>
		inline operator ObjT*()
		{
			return get_ObjectGroup<ObjT>.get_object_at(objectGroupID);
		}
	};
}

// object storage
namespace complex
{
	class	IObjectStorage
	{
	public: // interface
		virtual uint32_t	get_object_numInstances(	const uint32_t								OBJECT_INDEX) const = 0;

		virtual void		for_each_object_instance(	const uint32_t								OBJECT_INDEX,
														const std::function<void(const Entity&)>&	CALLBACK) = 0;
	};

	template<typename ObjT>
	class	ObjectStorage	: public IObjectStorage
							, protected dpl::DynamicOwner<ObjectGroup<ObjT>, ObjT>
					
	{
	private: // subtypes
		using	MyProperty	= dpl::Property<ObjectGroup<ObjT>, ObjT>;
		using	MyOwnerBase	= dpl::DynamicOwner<ObjectGroup<ObjT>, ObjT>;

	public: // relations
		friend	Member<ObjT>;
		friend	dpl::Owner<ObjectGroup<ObjT>, ObjT>;
		friend	MyProperty;
		friend	MyBase;

	public: // functions
		inline uint32_t		size() const
		{
			return MyOwnerBase::numProperties();
		}

		inline void			update_capacity()
		{
			if(MyOwnerBase::is_full())
			{
				static const uint32_t	INITIAL_CAPACITY	= 8;
				const uint32_t			DOUBLE_SIZE			= 2 * size();
				MyOwnerBase::reserve_properties((DOUBLE_SIZE > 0) ? DOUBLE_SIZE : INITIAL_CAPACITY);
			}
		}

		inline ObjT&		get_object_at(				const uint32_t								OBJECT_INDEX)
		{
			return MyOwnerBase::get_property_at(OBJECT_INDEX);
		}

		inline const ObjT&	get_object_at(				const uint32_t								OBJECT_INDEX) const
		{
			return MyOwnerBase::get_property_at(OBJECT_INDEX);
		}

		inline ObjT*		find_object(				const LabelType&							OBJECT_NAME)
		{
			return static_cast<ObjT*>(labeler.find_entry(OBJECT_NAME));
		}

		inline const ObjT*	find_object(				const LabelType&							OBJECT_NAME) const
		{
			return static_cast<const ObjT*>(labeler.find_entry(OBJECT_NAME));
		}

		template<typename FunctorT>
		inline void			for_each_object(			FunctorT&&									functor)
		{
			MyOwnerBase::for_each_property(functor);
		}

		template<typename FunctorT>
		inline void			for_each_object(			FunctorT&&									functor) const
		{
			MyOwnerBase::for_each_property(functor);
		}

	protected: // functions
		template<typename... CTOR>
		inline ObjT&		create_object_internal(		CTOR&&...									args)
		{
			return MyOwnerBase::create_property(std::forward<CTOR>(args)...);
		}

		inline void			destroy_object_internal(	const uint32_t								OBJECT_INDEX)
		{
			MyOwnerBase::destroy_property_at(OBJECT_INDEX);
		}
	};
}

// group specialization
namespace complex
{
	template<typename ObjT, ChildTypeList CHILD_TYPES> // Root objects (no parents)
	class	GroupSpecialization<ObjT, ParentTypes<>, CHILD_TYPES, LevelTypes<>> final : public ObjectStorage<ObjT>
	{
	private: // subtypes
		using	MyBase = ObjectStorage<ObjT>;

	public: // friends
		template<typename, ParentTypeList, ChildTypeList, LevelTypesList>
		friend class RelationNode; //<-- Calls direct/indirect functions.

	public: // subtypes
		template<typename LevelT>
		using	ParentRef	= ObjT::template ParentRef<LevelT>;

	public: // functions
		/*
			Direct object creation invalidates all commands.
			If you want to support undo/redu operations see@ complex_Commands.h.
		*/
		template<typename... CTOR>
		inline ObjT&	create_object_directly(		CTOR&&...		args)
		{
			Project::ref().clear_commands();
			return MyBase::create_object_internal(std::forward<CTOR>(args)...);
		}

	private: // functions
		template<typename... CTOR>
		inline ObjT&	create_object_indirectly(	CTOR&&...		args)
		{
			return MyBase::create_object_internal(std::forward<CTOR>(args)...);
		}
	};

	template<typename ObjT, typename... ParentTs, ChildTypeList CHILD_TYPES, typename... LevelTs> // Objects with parents
	class	GroupSpecialization<ObjT, ParentTypes<ParentTs...>, CHILD_TYPES, LevelTypes<LevelTs...>> final : public ObjectStorage<ObjT>
	{
	private: // subtypes
		using	MyBase = ObjectStorage<ObjT>;

	public: // subtypes
		template<typename LevelT>
		using	ParentRef		= ObjT::template ParentRef<LevelT>;

		template<typename LevelT>
		using	PossibleParents = typename LevelT::Parents;

		template<typename LevelT>
		using	SafeRef		= std::conditional_t< PossibleParents<LevelT>::template has_type<ObjT>(),
							/* TRUE */		ObjectHandle, // <-- Use when ObjT can be used as ParentT.
							/* FALSE */		ParentRef<LevelT>>;
	public: // friends
		template<typename, ParentTypeList, ChildTypeList, LevelTypesList>
		friend class RelationNode; //<-- Calls direct/indirect functions.

	public: // functions
		/*
			Direct object creation invalidates all commands.
			If you want to support undo/redu operations see@ complex_Commands.h.
		*/
		template<typename... CTOR>
		inline ObjT&	create_object_directly(		ParentRef<LevelTs>...		parents,
													CTOR&&...					args)
		{
			Project::ref().clear_commands();
			return GroupSpecialization::create_object_safely(parents..., std::forward<CTOR>(args)...);
		}

	private: // functions
		template<typename... CTOR>
		inline ObjT&	create_object_indirectly(	ParentRef<LevelTs>...		parents,
													CTOR&&...					args)
		{
			return GroupSpecialization::create_object_safely(parents..., std::forward<CTOR>(args)...);
		}

		template<typename... CTOR>
		inline ObjT&	create_object_safely(		SafeRef<LevelTs>...			parents,
													CTOR&&...					args)
		{
			// This function prevents reference/pointer invalidation if ParentT == ObjT.
			if constexpr (complex::has_relation<ObjT, ObjT>())
			{
				// SafeRef converts ObjT& and ObjT* to ObjectHandle which stores object by index.
				MyBase::update_capacity();//<-- Reallocation happens here.
			}

			// Convert ObjectHandle back to reference or pointer.
			return GroupSpecialization::create_object_final(parents..., std::forward<CTOR>(args)...);
		}

		template<typename... CTOR>
		inline ObjT&	create_object_final(		ParentRef<LevelTs>...		parents,
													CTOR&&...					args)
		{
			return MyBase::create_object_internal(std::forward<ParentRef<LevelTs>>(parents)..., std::forward<CTOR>(args)...);
		}
	};
}

// object group
namespace complex
{
	template<typename ObjT>
	using	GroupSpecializationType = GroupSpecialization<ObjT, typename ObjT::PARENT_TYPES, typename ObjT::CHILD_TYPES, typename Child<typename ObjT::PARENT_TYPES, ObjT>::Levels>;

	template<typename ObjT>
	class	ObjectGroup	final	: public GroupSpecializationType<ObjT>
								, public Project::Module<ObjectGroup<ObjT>>
								, public dpl::Singleton<ObjectGroup<ObjT>>
	{
	private: // subtypes
		using	MySpecBase		= GroupSpecializationType<ObjT>;
		using	MyModuleBase	= Module<Project, ObjectGroup<ObjT>>;

		static_assert(std::is_final_v<ObjT>, "ObjT must be defined as final.");

	public: // lifecycle
		CLASS_CTOR			ObjectGroup(				const Binding&								BINDING)
			: MyModuleBase(BINDING)
		{
			
		}

		CLASS_CTOR			ObjectGroup(				const ObjectGroup&							OTHER) = delete;

		CLASS_CTOR			ObjectGroup(				ObjectGroup&&								other) noexcept = delete;

		ObjectGroup&		operator=(					const ObjectGroup&							OTHER) = delete;

		ObjectGroup&		operator=(					ObjectGroup&&								other) noexcept = delete;

	private: // functions
		inline void			destroy_invalid_objects_and_instances()
		{
			uint32_t index = 0;
			while(index < size())
			{
				auto& object = MySpecBase::get_object_at(index);
				if(static_cast<Member<ObjT>&>(object).is_valid())
				{
					if constexpr (dpl::is_type_complete_v<Instance<ObjT>>)
					{
						object.destroy_invalid_instances();
					}					
					++index;
				}
				else // Note: Each object group will destroy its own invalid objects.
				{
					MySpecBase::destroy_object_internal(index);
				}
			}
		}

	public: // implementation
		virtual uint32_t	get_object_numInstances(	const uint32_t								OBJECT_INDEX) const final override
		{
			if constexpr (dpl::is_type_complete_v<Instance<ObjT>>)
			{
				return MySpecBase::get_object_at(OBJECT_INDEX).numInstances();
			}

			return 0;
		}

		virtual void		for_each_object_instance(	const uint32_t								OBJECT_INDEX,
														const std::function<void(const Entity&)>&	CALLBACK) final override
		{
			if constexpr (dpl::is_type_complete_v<Instance<ObjT>>)
			{
				const auto& OBJECT		= MySpecBase::get_object_at(OBJECT_INDEX);
				const auto* INSTANCES	= &OBJECT.get_first_instance();

				for(uint32_t index = 0; index < OBJECT.numInstances(); ++index)
				{
					CALLBACK(INSTANCES[index]);
				}
			}
		}

	private: // implementation
		virtual bool		on_event(					const dpl::Event&							EVENT) final override
		{
			if(EVENT.is_type<CommandsCleared>())
			{
				destroy_invalid_objects_and_instances();
				return true;
			}

			return false;
		}

		virtual void		on_destroy() final override
		{
			Project::ref().clear_commands();
			MySpecBase::for_each_object([](ObjT& object)
			{
				// TODO: Destroy object directly.
			});
		}
	};
}