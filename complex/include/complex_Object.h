#pragma once


#include <dpl_Command.h>
#include "complex_Components.h"
#include "complex_Member.h"


// declaration
namespace complex
{
	enum	ReservePolicy
	{
		AUTOMATIC_RESERVE,	// Reserve handled by the container. (Use when number of elements is large, or you used the container for a while)
		MANUAL_RESERVE		// Reserve to store exact number of elements. (Use when number of elements is small, e.g. initialization of the container)
	};


	template<typename ObjT, typename... ComponentTs>
	class	InstanceBase;

	/*
		Specialize for ObjT to enable instantiation.
		Must be derived from InstanceBase<ObjT, ComponentTs...>.
	*/
	template<typename ObjT>
	class	Instance;

	/*
		Base class for instantiable objects.
	*/
	template<typename ObjT, ParentTypeList PARENT_TYPES, ChildTypeList CHILD_TYPES, LevelTypesList LEVEL_TYPES>
	class	InstantiableObject;

	/*
		Base class for objects that cannot be instanced.
	*/
	template<typename ObjT, ParentTypeList PARENT_TYPES, ChildTypeList CHILD_TYPES, LevelTypesList LEVEL_TYPES>
	using	UniqueObject = RelationNode<ObjT, PARENT_TYPES, CHILD_TYPES, LEVEL_TYPES>;
}

// instantiable object
namespace complex
{
	/*
		Base class for Instance<ObjT>
	*/
	template<typename ObjT, typename... ComponentTs>
	class	InstanceBase : private dpl::Property<ObjT, Instance<ObjT>>
	{
	private: // subtypes
		using	MyBase	= dpl::Property<ObjT, Instance<ObjT>>;
		using	MyOwner	= dpl::DynamicOwner<ObjT, Instance<ObjT>>;

	public: // subtypes
		using	COMPONENT_TYPES = ComponentTypes<ComponentTs...>;

	public: // friends
		friend	MyBase;
		friend	MyOwner;
		
		template<typename, ParentTypeList, ChildTypeList, LevelTypesList, ComponentTypeList>
		friend class InstantiableObject;

	public: // data
		dpl::ReadOnly<dpl::Mask32_t, InstanceBase> flags; // [INTERNAL_FLAGS][USER_FLAGS] (same rules as Member::flags)

	public: // exposed functions
		using	MyBase::get_index;

	protected: // exposed functions
		using	MyBase::get_owner;
	
	protected: // CTORs and operators to override
		CLASS_CTOR					InstanceBase(		const dpl::Ownership&		OWNERSHIP)
			: MyBase(OWNERSHIP)
		{

		}

		CLASS_CTOR					InstanceBase(		const dpl::Ownership&		OWNERSHIP, 
														InstanceBase&&				other) noexcept
			: MyBase(OWNERSHIP, std::move(other))
			, flags(other.flags)
		{
			
		}

		inline InstanceBase&		operator=(			dpl::Swap<Instance<ObjT>>	other)
		{
			flags.swap(other->flags);
			return *this;
		}

	public: // functions
		template<typename ComponentT>
		inline const ComponentT&	get_component() const
		{
			return MyBase::get_owner().get_instance_components<ComponentT>()[MyBase::get_index()];
		}

		inline bool					is_valid() const
		{
			return !flags().any_of(InternalFlags::INVALID);
		}

	protected: // functions
		template<typename ComponentT>
		inline ComponentT&			get_component()
		{
			return MyBase::get_owner().get_instance_components<ComponentT>()[MyBase::get_index()];
		}

		inline void					set_user_flag(		const uint32_t				FLAG_INDEX,
														const bool					bSTATE)
		{
			if(FLAG_INDEX < FIRST_USER_FLAG_INDEX)
				throw dpl::GeneralException(this, __LINE__, "User flags start at: " + std::to_string(FLAG_INDEX));

			flags->set_at(FLAG_INDEX, bSTATE);
		}

	private: // functions
		inline void					set_state(			const bool					bSTATE)
		{
#ifdef _DEBUG
			if(!is_valid())
				throw dpl::GeneralException(this, __LINE__, "Instance already invalid.");
#endif // _DEBUG

			flags->set(InternalFlags::MARKED_AS_INVALID, !bSTATE);
		}

		inline void					set_parent_state(	const bool					bSTATE)
		{
			flags->set(InternalFlags::PARENT_INVALIDATED, !bSTATE);
		}
	};


	template<typename ObjT, ParentTypeList PARENT_TYPES, typename... ChildTs, typename... LevelTs> requires dpl::is_type_complete_v<Instance<ObjT>>
	class	InstantiableObject<ObjT, PARENT_TYPES, ChildTypes<ChildTs...>, LevelTypes<LevelTs...>>	
		: public  RelationNode<ObjT, PARENT_TYPES, ChildTypes<ChildTs...>, LevelTypes<LevelTs...>>
		, private dpl::DynamicOwner<ObjT, Instance<ObjT>>
		, private  Composite<typename Instance<ObjT>::COMPONENT_TYPES>
	{
	private: // subtypes
		using	MyRelationBase	= RelationNode<ObjT, PARENT_TYPES, ChildTypes<ChildTs...>, LevelTypes<LevelTs...>>;
		using	MyOwnerBase		= dpl::DynamicOwner<ObjT, Instance<ObjT>>;
		using	MyCompositeBase	= Composite<typename Instance<ObjT>::COMPONENT_TYPES>;

	public: // subtypes
		template<typename LevelT>
		using	ParentRef		= MyRelationBase::template ParentRef<LevelT>;

	public: // commands
		class	CreateInstances : public dpl::Command
		{
		private: // data
			dpl::ReadOnly<uint32_t,				AddInstances> objectIndex;
			dpl::ReadOnly<dpl::IndexRange<>,	AddInstances> range;

		public: // lifecycle
			CLASS_CTOR		CreateInstances(InstantiableObject&	object,
											const uint32_t		NUM_INSTANCES)
				: objectIndex(static_cast<Member<ObjT>&>(object).get_index())
			{
				if(object.add_instances_recursively(NUM_INSTANCES))
				{
					range->set_begin(object.numInstances());
					range->increase_back(NUM_INSTANCES);
				}
			}

		private: // implementation
			virtual bool	valid() const final override
			{
				InstantiableObject& object = ObjectGroup<ObjT>::ref().get_object_at(objectIndex());
				if(!object.is_valid()) return false;
				return !range().empty();
			}

			virtual void	execute() final override
			{
				InstantiableObject& object = ObjectGroup<ObjT>::ref().get_object_at(objectIndex());
				range().for_each([&](const uint32_t INDEX)
				{
					object.set_invalid_instance_recursively(INDEX, false);
				});
			}

			virtual void	unexecute() final override
			{
				InstantiableObject& object = ObjectGroup<ObjT>::ref().get_object_at(objectIndex());
				range().for_each([&](const uint32_t INDEX)
				{
					object.set_invalid_instance_recursively(INDEX, true);
				});
			}
		};

		class	CreateInstance : public CreateInstances
		{
		public: // lifecycle
			CLASS_CTOR	CreateInstance(InstantiableObject&	object)
				: CreateInstances(object, 1)
			{

			}
		};

		class	InvalidateInstance : public dpl::Command
		{
		private: // data
			dpl::ReadOnly<uint32_t,	InvalidateInstance>	objectIndex;
			dpl::ReadOnly<uint32_t,	InvalidateInstance>	instanceIndex;

		public: // lifecycle
			CLASS_CTOR		InvalidateInstance(	InstantiableObject&	object,
												const uint32_t		INSTANCE_INDEX)
				: objectIndex(static_cast<Member<ObjT>&>(object).get_index())
				, instanceIndex(INSTANCE_INDEX)
			{

			}

		private: // implementation
			virtual bool	valid() const final override
			{
				const auto& OBJECT = ObjectGroup<ObjT>::ref().get_object(objectIndex());
				if(!OBJECT.is_valid()) return false;
				if(OBJECT.numInstances() <= instanceIndex()) return false;
				if(!OBJECT.get_instance_at(instanceIndex()).is_valid()) return false;
				return !OBJECT.is_instance_inheritor();
			}

			virtual void	execute() final override
			{
				ObjectGroup<ObjT>::ref().get_object(objectIndex()).set_instance_state(instanceIndex(), false);
			}

			virtual void	unexecute() final override
			{
				ObjectGroup<ObjT>::ref().get_object(objectIndex()).set_instance_state(instanceIndex(), true);
			}
		};

		class	InvalidateAllInstances : public dpl::Command
		{
		private: // data
			dpl::ReadOnly<uint32_t,					InvalidateAllInstances> objectIndex;
			dpl::ReadOnly<std::vector<uint32_t>,	InvalidateAllInstances> instances;

		public: // lifecycle
			CLASS_CTOR		InvalidateAllInstances(const InstantiableObject& OBJECT)
				: objectIndex(static_cast<const Member<ObjT>&>(OBJECT).get_index())
			{
				if(OBJECT.is_valid() && !OBJECT.is_instance_inheritor())
				{
					for(uint32_t index = 0; index < OBJECT.numInstances(); ++index)
					{
						if(OBJECT.get_instance_at(index).is_valid())
							instances->push_back(index);
					}
				}
			}

		public: // implementation
			virtual bool	valid() const final override
			{
				return !instances().empty();
			}

			virtual void	execute() final override
			{
				auto& object = ObjectGroup<ObjT>::ref().get_object(objectIndex());
				for(const auto& INDEX : instances())
				{
					object.set_invalid_instance_recursively(INDEX, true);
				}
			}

			virtual void	unexecute() final override
			{
				auto& object = ObjectGroup<ObjT>::ref().get_object(objectIndex());
				for(const auto& INDEX : instances())
				{
					object.set_invalid_instance_recursively(INDEX, false);
				}
			}
		};

	public: // friends
		friend	CreateInstances;
		friend	InvalidateInstance;
		friend	InvalidateAllInstances;
		friend	ObjectGroup<ObjectT>;

	public: // data
		dpl::ReadOnly<uint32_t, InstantiableObject> numInvalidInstances;

	public: // inherited functions
		using	MyCompositeBase::set_component_usage_to_CPU;
		using	MyCompositeBase::set_component_usage_to_GPU;
		using	MyCompositeBase::has_components;
		using	MyCompositeBase::get_components;

	protected: // lifecycle
		CLASS_CTOR						InstantiableObject(					const dpl::Ownership&			OWNERSHIP,
																			const dpl::Mask32<Flags>		FLAGS,
																			ParentRef<LevelTs>...			parent)
			: MyRelationBase(OWNERSHIP, FLAGS, parent...)
			, numInvalidInstances(0)
		{
			InstantiableObject::initialize_instances(FLAGS, parent...);
		}

		CLASS_CTOR						InstantiableObject(					const dpl::Ownership&			OWNERSHIP,
																			InstantiableObject&&			other) noexcept
			: MyRelationBase(OWNERSHIP, std::move(other))
			, MyOwnerBase(std::move(other))
			, MyCompositeBase(std::move(other))
			, numInvalidInstances(other.numInvalidInstances)
		{

		}

		CLASS_CTOR						InstantiableObject(					const InstantiableObject&		OTHER) = delete;

		CLASS_CTOR						InstantiableObject(					InstantiableObject&&			other) noexcept = delete;

		InstantiableObject&				operator=(							const InstantiableObject&		OTHER) = delete;

		InstantiableObject&				operator=(							InstantiableObject&&			other) noexcept = delete;

		InstantiableObject&				operator=(							dpl::Swap<InstantiableObject>	other)
		{
			MyRelationBase::operator=(dpl::Swap<MyRelationBase>(*other));
			MyOwnerBase::operator=(dpl::Swap<MyOwnerBase>(*other));
			MyCompositeBase::operator=(dpl::Swap<MyCompositeBase>(*other));
			numInvalidInstances.swap(other->numInvalidInstances);
			return *this;
		}

	public: // functions
		/*
			Returns true if object has at least one parent type and INHERITED_INSTANCES flag is set to true.
		*/
		inline bool						is_instance_inheritor() const
		{
			if constexpr (PARENT_TYPES::SIZE == 0) return false;
			return Member<ObjT>::flags().fits(Flags::INHERITED_INSTANCES);
		}

		/*
			Returns total number of instances (both valid and invalid).
		*/
		inline uint32_t					numInstances() const
		{
			return MyOwnerBase::numProperties();
		}

		inline uint32_t					numValidInstances() const
		{
			return numInstances() - numInvalidInstances();
		}

		inline bool						has_instances() const
		{
			return numInstances() > 0;
		}

		inline Instance<ObjT>&			get_instance_at(					const uint32_t					INDEX)
		{
			return MyOwnerBase::get_property_at(INDEX);
		}

		inline const Instance<ObjT>&	get_instance_at(					const uint32_t					INDEX) const
		{
			return MyOwnerBase::get_property_at(INDEX);
		}

		inline Instance<ObjT>&			get_first_instance()
		{
			return MyOwnerBase::get_first_property();
		}

		inline const Instance<ObjT>&	get_first_instance() const
		{
			return MyOwnerBase::get_first_property();
		}

		inline Instance<ObjT>&			get_last_instance()
		{
			return MyOwnerBase::get_last_property();
		}

		inline const Instance<ObjT>&	get_last_instance() const
		{
			return MyOwnerBase::get_last_property();
		}

		template<typename FunctorT>
		inline void						for_each_instance(					FunctorT&&						functor)
		{
			MyOwnerBase::for_each_property(functor);
		}

		template<typename FunctorT>
		inline void						for_each_instance(					FunctorT&&						functor) const
		{
			MyOwnerBase::for_each_property(functor);
		}

	private: // internal functions
		inline bool						reserve_instances_internal(			const uint32_t					CAPACITY)
		{
			if(!MyOwnerBase::reserve_properties(CAPACITY)) return false;
			MyCompositeBase::reserve_components(CAPACITY);
			return true;
		}

		bool							add_instances_internal(				uint32_t						numInstances,
																			const ReservePolicy				RESERVE_POLICY = ReservePolicy::AUTOMATIC_RESERVE)
		{
			if(numInstances == 0) return false;
			MyCompositeBase::add_components(numInstances);
			if(RESERVE_POLICY == ReservePolicy::MANUAL_RESERVE) MyOwnerBase::reserve_properties(numInstances() + numInstances);
			while(numInstances > 0) {MyOwnerBase::create_property(); --numInstances;}
			return true;
		}

		inline void						destroy_instance_internal(			const uint32_t					INSTANCE_ID)
		{
			if(!MyOwnerBase::destroy_property_at(INSTANCE_ID)) return;
			MyCompositeBase::remove_components_at(INSTANCE_ID);
		}

		inline void						destroy_all_instances_internal()
		{
			if(!MyOwnerBase::destroy_all_properties()) return;
			MyCompositeBase::remove_all_components();
		}

		void							destroy_invalid_instances()
		{
			// NOTE: This function is called for all objects in the project, no need to call it recursively.
			uint32_t index = 0;
			while(index < numInstances())
			{
				if(get_instance_at(index).is_valid()) ++index;
				else destroy_instance_internal(index); //<-- Swaps with the last instance, DO NOT increment index!
			}
		}

	private: // recursive functions
		inline void						validate_inheritor(					const bool						bSUCCESS) const
		{
#ifdef _DEBUG
			if(!bSUCCESS) 
				throw dpl::GeneralException(this, __LINE__, "Inheritor failure!");
#endif //_DEBUG
		}

		/*
			INVOKE must return bool value.
			Throws GeneralException on false (DEBUG mode only).
		*/
		template<typename ChildT, typename FunctorT>
		inline void						invoke_instance_inheritors(			FunctorT&&						INVOKE)
		{
			if constexpr (MyRelationBase::template is_primary_parent_of<ChildT>() && dpl::is_type_complete_v<Instance<ChildT>>)
			{
				MyRelationBase::template for_each_child<ChildT>([&](ChildT& child)
				{
					if(child.is_instance_inheritor())
						validate_inheritor(INVOKE(child));
				});
			}
		}

		inline bool						reserve_instances_recursively(		const uint32_t					CAPACITY)
		{
			if(!reserve_instances_internal(CAPACITY)) return false;
			(InstantiableObject::invoke_instance_inheritors<ChildTs>([&](ChildTs& child)
			{
				if constexpr (dpl::is_type_complete_v<Instance<ChildTs>>)
					return child.reserve_instances_recursively(CAPACITY);
			}), ...);
			return true;
		}

		inline bool						add_instances_recursively(			const uint32_t					NUM_INSTANCES)
		{
			if(!add_instances_internal(NUM_INSTANCES)) return false;
			(InstantiableObject::invoke_inheritors<ChildTs>([&](ChildTs& child)
			{
				if constexpr (dpl::is_type_complete_v<Instance<ChildTs>>)
					return child.add_instances_recursively(NUM_INSTANCES);

			}), ...);
			return true;
		}

	private: // instance invalidation
		inline void						update_numInvalidInstances(			const bool						bSTATE)
		{
			bSTATE? --(*numInvalidInstances) : ++(*numInvalidInstances);
		}

		inline void						notify_parent_instance_state(		const uint32_t					INSTANCE_ID,
																			const bool						bSTATE)
		{
			(MyParentBase::template for_each_child<ChildTs>([&](ChildTs& child)
			{
				if constexpr (dpl::is_type_complete_v<Instance<ChildTs>>)
				{
					auto& instance = child.get_instance_at(INSTANCE_ID);
					const bool bWAS_VALID = instance.is_valid();
					instance.set_parent_state(bSTATE);
					if(bWAS_VALID != instance.is_valid())
						child.update_numInvalidInstances(bSTATE);
					child.notify_parent_instance_state(INSTANCE_ID, bSTATE);
				}
			}), ...);
		}

		inline void						set_instance_state(					const uint32_t					INSTANCE_ID,
																			const bool						bSTATE)
		{
			get_instance_at(INSTANCE_ID).set_state(bSTATE);
			update_numInvalidInstances(bSTATE);
			InstantiableObject::notify_parent_instance_state(INSTANCE_ID, bSTATE);
		}

	private: // parent instance info extraction
		template<uint32_t LEVEL_ID>
		inline void						initialize_instances(				const Origin<ObjT, LEVEL_ID>&	ORIGIN)
		{
			// [PROBLEM]: New instances may be initially invalid! How do we test that?!
			ORIGIN.NUM_INSTANCES;
		}

		template<uint32_t LEVEL_ID, typename ParentT>
		inline void						initialize_instances(				ParentT&						parent)
		{
			if constexpr(dpl::is_type_complete_v<Instance<ParentT>>)
			{
				add_instances_internal(parent.numInstances(), ReservePolicy::MANUAL_RESERVE);
				for(uint32_t index = 0; index < parent.numInstances(); ++index)
				{
					if(parent.get_instance_at(index).is_valid()) continue;
					get_instance_at(index).set_parent_state(false);
					++(*numInvalidInstances);
				}
			}
		}

		inline void						initialize_instances(				const dpl::Mask32<Flags>		FLAGS)
		{
			// Object has no parent and therefore no initial instances.
		}

		template<typename LevelT>
		inline void						initialize_instances(				const dpl::Mask32<Flags>		FLAGS,
																			ParentRef<LevelT>				parent)
		{
			if(!FLAGS.at(INHERITED_INSTANCES_INDEX)) return;
			InstantiableObject::initialize_instances<LevelT::LEVEL>(parent);
		}

		template<typename FirstLevelT, typename... OtherLevelTs>
		inline void						initialize_instances(				const dpl::Mask32<Flags>		FLAGS,
																			ParentRef<FirstLevelT>			firstParent,
																			ParentRef<OtherLevelTs>...		otherParent)
		{
			InstantiableObject::initialize_instances<FirstLevelT>(FLAGS, firstParent);
		}
	};
}

// initializer implementation
namespace complex
{
	template<typename ParentT, typename ChildT>
	class RuntimeInitializer final : public IRuntimeInitializer<ChildT, complex::get_RelationLevel<ParentT, ChildT>()>
	{
	public: // constants
		static constexpr uint32_t	LEVEL		= complex::get_RelationLevel<ParentT, ChildT>();
		static constexpr bool		bOPTIONAL	= complex::is_OPTIONAL_relation<ChildT, LEVEL>();

	public: // subtypes
		using ParentRef = std::conditional_t<bOPTIONAL, ParentT*, ParentT&>;

	public: // data
		dpl::ReadOnly<uint32_t, RuntimeInitializer> objectID;

	private: // lifeycle
		CLASS_CTOR								RuntimeInitializer(	ParentT*	parent, bool dummy)
			: objectID(parent->get_index())
		{

		}

		CLASS_CTOR								RuntimeInitializer(	ParentT&	parent, bool dummy)
			: objectID(parent.get_index())
		{

		}

	public: // lifecycle
		CLASS_CTOR								RuntimeInitializer(	ParentRef	parent)
			: RuntimeInitializer(parent, true)
		{

		}

	public: // implementation
		virtual RuntimeParent<ChildT, LEVEL>*	get_parent() final override
		{
			return &ObjectGroup<ParentT>::ref().get_object_at(objectID);
		}

		virtual uint32_t						get_numInstances() const final override
		{
			if constexpr (dpl::is_type_complete_v<Instance<ParentT>>)
			{
				return ObjectGroup<ParentT>::ref().get_object_at(objectID).numInstances();
			}
			
			return 0;
		}

		virtual uint32_t						get_parentTypeID() const
		{
			return ChildT::template get_parentTypeID<ParentT>();
		}

		virtual void							initialize_instances(const std::function<void(const bool bINSTANCE_STATE)>&) const
		{
			auto& parent = ObjectGroup<ParentT>::ref().get_object_at(objectID);
			parent.for_each_instance()
		}
	};
}

// final layer for complex objects
namespace complex
{
/*
	Defines data structure of the child and its parents for the specific relation level.
	Relation level allows a child to have more than one parent at a time.
	Any number can be used for the level. 
	The level is required to distinguish between dpl::Sequenceable(internal interface) of the same child type, 
	but it is also used with dpl::Association for simplicity of the templates,
	so be careful when using it together with dpl patterns.
	Must be used within complex namespace.
*/
#define DECLARE_COMPLEX_RELATION(_CHILD_, _LEVEL_, _REQUIREMENT_, _TYPE_)		\
	template<> struct RelationRequirementTag<_CHILD_, _REQUIREMENT_, _LEVEL_>{};\
	template<> struct RelationTypeTag<_CHILD_, _TYPE_, _LEVEL_>{}

/*
	Defines relation between parent and child types.
	Setting more than one parent type per level results in an additional 4B overhead for that level(RTT information).
	complex::UnknownParent proxy type can be used to allow for any type of parent, but only one can be used for all relation levels of a single child type.
	Must be used within complex namespace.
*/
#define EXPAND_COMPLEX_RELATION(_CHILD_, _LEVEL_, _PARENT_)													\
	template<> struct PairTag<_PARENT_, _CHILD_>{															\
		static constexpr RelationRequirement	REQUIREMENT = get_RelationRequirement<_CHILD_, _LEVEL_>();	\
		static constexpr RelationType			TYPE		= get_RelationType<_CHILD_, _LEVEL_>();			\
		static constexpr uint32_t				LEVEL		= _LEVEL_;}


	template<typename ObjT, ParentTypeList PARENT_TYPES, ChildTypeList CHILD_TYPES>
	using	Object = std::conditional_t<dpl::is_type_complete_v<Instance<ObjT>> // Instance defined?
						/*TRUE*/,	InstantiableObject<ObjT, PARENT_TYPES, CHILD_TYPES, typename Child<PARENT_TYPES, ObjT>::Levels>
						/*FALSE*/,	UniqueObject<ObjT, PARENT_TYPES, CHILD_TYPES, typename Child<PARENT_TYPES, ObjT>::Levels>>;
}