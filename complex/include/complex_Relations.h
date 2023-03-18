#pragma once


#include <memory>
#include <dpl_Chain.h>
#include <dpl_TypeTraits.h>
#include <dpl_Command.h>
#include "complex_Utilities.h"


#pragma pack(push, 4)
// requirement
namespace complex
{
	enum	RelationRequirement
	{
		OBLIGATORY, // Child needs parent of given relation level to exist.
		OPTIONAL	// Child is not dependant on given relation level. Note: parent dependency is NOT imported/exported.
	};


	template<typename ChildT, RelationRequirement REQUIREMENT, uint32_t LEVEL>
	class	RelationRequirementTag;


	template<typename ChildT, uint32_t LEVEL>
	constexpr RelationRequirement	get_RelationRequirement()
	{
		constexpr bool bOBLIGATORY	= dpl::is_type_complete_v<RelationRequirementTag<ChildT, RelationRequirement::OBLIGATORY, LEVEL>>;
		constexpr bool bOPTIONAL	= dpl::is_type_complete_v<RelationRequirementTag<ChildT, RelationRequirement::OPTIONAL, LEVEL>>;

		if constexpr(bOBLIGATORY)		
		{
			static_assert(!bOPTIONAL, "Fail to set OBLIGATORY relation: OPTIONAL already selected.");
			return RelationRequirement::OBLIGATORY;
		}
		else if constexpr(bOPTIONAL)	
		{
			static_assert(!bOBLIGATORY, "Fail to set OPTIONAL relation: OBLIGATORY already selected.");
			return RelationRequirement::OPTIONAL;
		}
		else // compile-time error (NOTE: We can't simply use false in static_assert, because it will always fail.)
		{
			static_assert(bOPTIONAL, "RelationType not defined.");
			return RelationRequirement::OPTIONAL; // dummy return type
		}
	}

	template<typename ChildT, uint32_t LEVEL>
	constexpr bool					is_OBLIGATORY_relation()
	{
		return complex::get_RelationRequirement<ChildT, LEVEL>() == RelationRequirement::OBLIGATORY;
	}

	template<typename ChildT, uint32_t LEVEL>
	constexpr bool					is_OPTIONAL_relation()
	{
		return complex::get_RelationRequirement<ChildT, LEVEL>() == RelationRequirement::OPTIONAL;
	}
}

// type
namespace complex
{
	/*
		Defines data structure of the child and its parents.
	*/
	enum	RelationType
	{
		ONE_TO_ONE,	// One parent type <-> One child type, [OVERHEAD]: 8B for both types
		ONE_TO_MANY,// One parent type <-> Many children of the same type, [OVERHEAD]: 20B for parent type, 24B for child type
	};


	template<typename ChildT, RelationType TYPE, uint32_t LEVEL>
	class	RelationTypeTag;


	template<typename ChildT, uint32_t LEVEL>
	constexpr RelationType	get_RelationType()
	{
		constexpr bool bONE_TO_ONE	= dpl::is_type_complete_v<RelationTypeTag<ChildT, RelationType::ONE_TO_ONE, LEVEL>>;
		constexpr bool bONE_TO_MANY	= dpl::is_type_complete_v<RelationTypeTag<ChildT, RelationType::ONE_TO_MANY, LEVEL>>;

		if constexpr(bONE_TO_ONE)		
		{
			static_assert(!bONE_TO_MANY, "Fail to set ONE_TO_ONE relation: ONE_TO_MANY already selected.");
			return RelationType::ONE_TO_ONE;
		}
		else if constexpr(bONE_TO_MANY)	
		{
			static_assert(!bONE_TO_ONE, "Fail to set ONE_TO_MANY relation: ONE_TO_ONE already selected.");
			return RelationType::ONE_TO_MANY;
		}
		else // compile-time error (NOTE: We can't simply use false in static_assert, because it will always fail.)
		{
			static_assert(bONE_TO_MANY, "RelationType not defined.");
			return RelationType::ONE_TO_MANY; // dummy return type
		}
	}

	template<typename ChildT, uint32_t LEVEL>
	constexpr bool			is_ONE_TO_ONE_relation()
	{
		return complex::get_RelationType<ChildT, LEVEL>() == RelationType::ONE_TO_ONE;
	}

	template<typename ChildT, uint32_t LEVEL>
	constexpr bool			is_ONE_TO_MANY_relation()
	{
		return complex::get_RelationType<ChildT, LEVEL>() == RelationType::ONE_TO_MANY;
	}
}

// parent-child pairs
namespace complex
{
	template<typename ParentT, typename ChildT>
	struct	PairTag;


	template<typename ParentT, typename ChildT>
	constexpr bool		has_relation()
	{
		if constexpr (dpl::is_type_complete_v<PairTag<ParentT, ChildT>>)
		{
			if constexpr (dpl::is_type_complete_v<decltype(PairTag<ParentT, ChildT>::LEVEL)>)
			{
				return std::is_same_v<decltype(PairTag<ParentT, ChildT>::LEVEL), const uint32_t>;
			}
		}

		return false;
	}

	template<typename ParentT, typename ChildT>
	constexpr uint32_t	get_RelationLevel()
	{
		constexpr bool bDEFINED = complex::has_relation<ParentT, ChildT>();
		static_assert(bDEFINED, "Undefined relation level.");
		if constexpr (bDEFINED) return PairTag<ParentT, ChildT>::LEVEL;
		return 0;
	}
}

// parent declarations
namespace complex
{
	template<typename... Ts>
	using	ChildTypes		= dpl::TypeList<Ts...>;

	template<typename TList>
	concept	ChildTypeList	= dpl::is_specialization_v<TList, dpl::TypeList>;

	template<typename ParentT, typename ChildT, RelationType TYPE, uint32_t LEVEL>
	class	IParent;

	template<typename ParentT, ChildTypeList CHILD_TYPES>
	class	ParentWithManyChildren;
}

// child declarations
namespace complex
{
	template<typename... Ts>
	using	ParentTypes		= dpl::TypeList<Ts...>;

	template<typename TList>
	concept	ParentTypeList	= dpl::is_specialization_v<TList, dpl::TypeList>;

	template<typename... Ts>
	using	LevelTypes		= dpl::TypeList<Ts...>;

	template<typename TList>
	concept	LevelTypesList	= dpl::is_specialization_v<TList, dpl::TypeList>;

	template <typename ParentT, typename ChildT, RelationType TYPE, uint32_t LEVEL>
	class	IChild;

	template <ParentTypeList PARENT_TYPES, typename ChildT>
	class	Child;
}

// child interface
namespace complex
{
	template <typename ParentT, typename ChildT, uint32_t LEVEL>
	class	IChild<ParentT, ChildT, RelationType::ONE_TO_ONE, LEVEL> : private dpl::Association<ChildT, ParentT, LEVEL>
	{
	private: // subtypes
		using MyBase	= dpl::Association<ChildT, ParentT, LEVEL>;
		using MyParent	= IParent<ParentT, ChildT, RelationType::ONE_TO_ONE, LEVEL>;

	public: // friends
		friend MyParent;

		template<ParentTypeList, typename>
		friend class Child;

		template<typename, typename, uint32_t>
		friend class dpl::Association;

	protected: // lifecycle
		CLASS_CTOR				IChild(		MyParent*			parent)
		{
			if(parent) parent->add_child(static_cast<ChildT&>(*this));
		}

		CLASS_CTOR				IChild(		const IChild&		OTHER) = delete;
		CLASS_CTOR				IChild(		IChild&&			other) noexcept = default;

		IChild&					operator=(	const IChild&		OTHER) = delete;
		IChild&					operator=(	IChild&&			other) noexcept = default;

		inline IChild&			operator=(	dpl::Swap<IChild>	other) noexcept
		{
			MyBase::operator=(dpl::Swap<MyBase>(*other));
			return *this;
		}

	protected: // functions
		inline bool				has_parent() const
		{
			return MyBase::is_linked();
		}

		inline ParentT&			get_parent()
		{
			return *MyBase::other();
		}

		inline const ParentT&	get_parent() const
		{
			return *MyBase::other();
		}

		inline ChildT*			previous_sibling()
		{
			return nullptr;
		}

		inline const ChildT*	previous_sibling() const
		{
			return nullptr;
		}

		inline ChildT*			next_sibling()
		{
			return nullptr;
		}

		inline const ChildT*	next_sibling() const
		{
			return nullptr;
		}
	};

	template <typename ParentT, typename ChildT, uint32_t LEVEL>
	class	IChild<ParentT, ChildT, RelationType::ONE_TO_MANY, LEVEL> : private dpl::Link<ParentT, ChildT, LEVEL>
	{
	private: // subtypes
		using MyParent		= IParent<ParentT, ChildT, RelationType::ONE_TO_MANY, LEVEL>;
		using MyChain		= dpl::Chain<ParentT, ChildT, LEVEL>;
		using MyLinkBase	= dpl::Link<ParentT, ChildT, LEVEL>;

	public: // friends
		friend MyParent;
		friend MyChain;
		friend MyLinkBase;
		friend dpl::Sequenceable<ChildT, LEVEL>;

		template<ParentTypeList, typename>
		friend class Child;

	protected: // lifecycle
		CLASS_CTOR				IChild(		MyParent*			parent)
		{
			if(parent) parent->add_child(static_cast<ChildT&>(*this));
		}

		CLASS_CTOR				IChild(		const IChild&		OTHER) = delete;
		CLASS_CTOR				IChild(		IChild&&			other) noexcept = default;

		IChild&					operator=(	const IChild&		OTHER) = delete;
		IChild&					operator=(	IChild&&			other) noexcept = default;

		inline IChild&			operator=(	dpl::Swap<IChild>	other) noexcept
		{
			MyLinkBase::operator=(dpl::Swap<MyLinkBase>(*other));
			return *this;
		}

	private: // functions
		inline bool				has_parent() const
		{
			return MyLinkBase::is_linked();
		}

		inline ParentT&			get_parent()
		{
			return *MyLinkBase::get_chain();
		}

		inline const ParentT&	get_parent() const
		{
			return *MyLinkBase::get_chain();
		}

		inline ChildT*			previous_sibling()
		{
			return MyLinkBase::previous();
		}

		inline const ChildT*	previous_sibling() const
		{
			return MyLinkBase::previous();
		}

		inline ChildT*			next_sibling()
		{
			return MyLinkBase::next();
		}

		inline const ChildT*	next_sibling() const
		{
			return MyLinkBase::next();
		}
	};
}

// parent interface
namespace complex
{
	template <typename ParentT, typename ChildT, uint32_t LEVEL>
	class	IParent<ParentT, ChildT, RelationType::ONE_TO_ONE, LEVEL> : private dpl::Association<ParentT, ChildT, LEVEL>
	{
	private: // subtypes
		using	MyChild	= IChild<ParentT, ChildT, RelationType::ONE_TO_ONE, LEVEL>;
		using	MyBase	= dpl::Association<ParentT, ChildT, LEVEL>;

	public: // subtypes
		using	InvokeChild			= std::function<void(ChildT&)>;
		using	InvokeConstChild	= std::function<void(const ChildT&)>;

	public: // friends
		friend	MyChild;

		template<typename, ChildTypeList>
		friend class Parent;

		template<typename, typename, uint32_t>
		friend class dpl::Association;

	private: // functions
		inline uint32_t			get_numChildren() const
		{
			return MyBase::is_linked() ? 1 : 0;
		}

		inline bool				can_have_another_child() const
		{
			return !MyBase::is_linked();
		}

		inline void				add_child(				ChildT&					child)
		{
			if(!can_have_another_child())
				throw dpl::GeneralException(this, __LINE__, "Could not add another child of this type.");

			if(!MyBase::link(child))
				throw dpl::GeneralException(this, __LINE__, "Child already belongs to this parent.");
		}

		inline bool				remove_child(			ChildT&					child)
		{
			return MyBase::unlink(child);
		}

		inline ChildT*			get_first_child()
		{
			return MyBase::other();
		}

		inline const ChildT*	get_first_child() const
		{
			return MyBase::other();
		}

		inline void				for_each_child(			const InvokeChild&		INVOKE)
		{
			if(MyBase::is_linked()) INVOKE(*MyBase::other());
		}

		inline void				for_each_child(			const InvokeConstChild&	INVOKE) const
		{
			if(MyBase::is_linked()) INVOKE(*MyBase::other());
		}

		template<typename ComparatorT>
		inline void				sort(					ComparatorT&&)
		{
			// This is a dummy function to match member function from other specialization.
		}
	};

	template <typename ParentT, typename ChildT, uint32_t LEVEL>
	class	IParent<ParentT, ChildT, RelationType::ONE_TO_MANY, LEVEL> : private dpl::Chain<ParentT, ChildT, LEVEL>
	{
	private: // subtypes
		using	MyChild				= IChild<ParentT, ChildT, RelationType::ONE_TO_MANY, LEVEL>;
		using	MyChainBase			= dpl::Chain<ParentT, ChildT, LEVEL>;
		using	MyLink				= dpl::Link<ParentT, ChildT, LEVEL>;

	public: // subtypes
		using	InvokeChild			= std::function<void(ChildT&)>;
		using	InvokeConstChild	= std::function<void(const ChildT&)>;

	public: // friends
		friend	MyChild;
		friend	MyChainBase;
		friend	MyLink;

		template<typename, ChildTypeList>
		friend class Parent;

	protected: // lifecycle
		CLASS_CTOR				IParent() = default;
		CLASS_CTOR				IParent(				const IParent&			OTHER) = delete;
		CLASS_CTOR				IParent(				IParent&&				other) noexcept = default;

		IParent&				operator=(				const IParent&			OTHER) = delete;
		IParent&				operator=(				IParent&&				other) noexcept = default;
		inline IParent&			operator=(				dpl::Swap<IParent>		other) noexcept
		{
			MyChainBase::operator=(dpl::Swap<MyChainBase>(*other));
			return *this;
		}

	private: // functions
		inline uint32_t			get_numChildren() const
		{
			return MyChainBase::size();
		}

		inline bool				can_have_another_child() const
		{
			return true;
		}

		inline void				add_child(				ChildT&					child)
		{
			if(!MyChainBase::attach_back(child))
				throw dpl::GeneralException(this, __LINE__, "Child already belongs to this parent."); //<-- Should never happen.
		}

		inline bool				remove_child(			ChildT&					child)
		{
			return MyChainBase::detach_link(child);
		}

		inline ChildT*			get_first_child()
		{
			return MyChainBase::first();
		}

		inline const ChildT*	get_first_child() const
		{
			return MyChainBase::first();
		}

		inline void				for_each_child(			const InvokeChild&		INVOKE)
		{
			MyChainBase::for_each_link(INVOKE);
		}

		inline void				for_each_child(			const InvokeConstChild&	INVOKE) const
		{
			MyChainBase::for_each_link(INVOKE);
		}
	};

	template<typename ChildT, uint32_t LEVEL>
	class	RuntimeParent;

	template<typename ChildT, uint32_t LEVEL>
	using	RuntimeParentBase = IParent<RuntimeParent<ChildT, LEVEL>, ChildT, complex::get_RelationType<ChildT, LEVEL>(), LEVEL>;

	/*
		This type is used in between ParentT and IParent, as an abstraction level when ParentT cannot be known at compile time.
	*/
	template<typename ChildT, uint32_t LEVEL>
	class	RuntimeParent : public RuntimeParentBase<ChildT, LEVEL>
	{
	private: // subtypes
		using MyBase = RuntimeParentBase<ChildT, LEVEL>;

	protected: // lifecycle
		CLASS_CTOR			RuntimeParent() = default;
		CLASS_CTOR			RuntimeParent(		const RuntimeParent&		OTHER) = delete;
		CLASS_CTOR			RuntimeParent(		RuntimeParent&&				other) noexcept = default;

		RuntimeParent&		operator=(			const RuntimeParent&		OTHER) = delete;
		RuntimeParent&		operator=(			RuntimeParent&&				other) noexcept = default;
		RuntimeParent&		operator=(			dpl::Swap<RuntimeParent>	other) noexcept
		{
			MyBase::operator=(dpl::Swap<MyBase>(*other));
			return *this;
		}
	};
}

// instance initializer
namespace complex
{
	template<typename ObjT>
	class ObjectGroup;

	template<typename ChildT, uint32_t LEVEL>
	class IRuntimeInitializer
	{
	public: // interface
		virtual									~IRuntimeInitializer() = default;

		virtual RuntimeParent<ChildT, LEVEL>*	get_parent() = 0;

		virtual uint32_t						get_numInstances() const = 0;

		virtual uint32_t						get_parentTypeID() const = 0;

		virtual void							initialize_instances(const std::function<void(const bool bINSTANCE_STATE)>&) const = 0;
	};
}

// origin
namespace complex
{
	static const uint32_t INVALID_ID = std::numeric_limits<uint32_t>::max();

	/* 
		Use this tag to define unknown origin.
	*/
	struct	UnknownParent;

	template<typename ParentT, typename ChildT, uint32_t LEVEL>
	concept IsRuntimeParent = std::is_base_of_v<RuntimeParent<ChildT, LEVEL>, ParentT>;

	/*
		Store typeID of the Group<ObjT> and indexof the object?

		Create Fabric-like class that accepts Origin and initializes the Child?

		1) Pass list of parents/origins to the Group<ObjT>
		2) Convert all references to origins.
		3) Reserve space for new ObjT.
		4) Pass converted list of references to the ObjT constructor. (Origin must have private conversion operator that cast it to any ObjT& and ObjT*)
	*/
	template<typename ChildT, uint32_t LEVEL, bool bOPTIONAL>
	class	OriginBase;

	template<typename ChildT, uint32_t LEVEL>
	class	OriginBase<ChildT, LEVEL, true>
	{
	public: // friends
		template<ParentTypeList, typename>
		friend class Child;

	private: // data
		std::unique_ptr<IRuntimeInitializer<ChildT, LEVEL>>	initializer;

	public: // lifecycle
		CLASS_CTOR								OriginBase(		std::nullptr_t)
		{
			
		}

		template<typename ParentT> requires IsRuntimeParent<ParentT, ChildT, LEVEL>
		CLASS_CTOR								OriginBase(		ParentT*	parent)
			: parent(parent)
		{
			
		}

		IRuntimeInitializer<ChildT, LEVEL>*		operator->()
		{
			return initializer.get();
		}
	};

	template<typename ChildT, uint32_t LEVEL>
	class	OriginBase<ChildT, LEVEL, false>
	{
	public: // friends
		template<ParentTypeList, typename>
		friend class Child;

	private: // data
		RuntimeParent<ChildT, LEVEL>*	parent;

	public: // data
		const uint32_t					TYPE_ID;		// Index of the parent type within parent type list of a specific relation level.
		const uint32_t					NUM_INSTANCES;	// Current number of instances of the parent object, or 0 if not instantiable.

	public: // lifecycle
		template<typename ParentT> requires IsRuntimeParent<ParentT, ChildT, LEVEL>
		CLASS_CTOR				OriginBase(		ParentT&	parent)
			: parent(&parent)
			, TYPE_ID(ChildT::template get_parentTypeID<ParentT>()) //<-- Defined in Child::Type
			, NUM_INSTANCES(0) // TODO
		{
			
		}
	};

	/*
		This class acts as a wrapper of the RTT parent.
		Passed to the child constructor in place of the ParentT.
	*/
	template<typename ChildT, uint32_t LEVEL>
	using	Origin = OriginBase<ChildT, LEVEL, complex::is_OPTIONAL_relation<ChildT, LEVEL>()>;
}

// child implementation
namespace complex
{
	template<ParentTypeList PARENT_TYPES, typename ChildT>
	class	Child
	{
	public: // friends
		template<typename, ChildTypeList>
		friend class Parent;

	public: // subtypes
		template<uint32_t _LEVEL_>
		struct	MyRelation
		{
		private: // subtypes
			template<typename ParentT>
			struct	parent_is_member
			{
				static constexpr bool value = complex::get_RelationLevel<ParentT, ChildT>() == _LEVEL_;
			};

		public: // subtypes
			using	Parents = PARENT_TYPES::template Subtypes<parent_is_member>;
			using	OriginT = Origin<ChildT, _LEVEL_>;

		public: // constants
			static constexpr uint32_t	LEVEL			= _LEVEL_;
			static constexpr bool		bRUNTIME_PARENT	= Parents::SIZE > 1;
		};

	private: // subtypes
		template<typename ParentA, typename ParentB>
		struct	is_same_level
		{
			static constexpr bool value = complex::get_RelationLevel<ParentA, ChildT>() == complex::get_RelationLevel<ParentB, ChildT>();
		};

		template<ParentTypeList PARENT_TYPES_X>
		struct	is_sorted
		{
			static constexpr bool value = true;
		};

		template<typename ParentT, typename NextParentT>
		struct	is_sorted<dpl::TypeList<ParentT, NextParentT>>
		{
			static constexpr bool value = complex::get_RelationLevel<ParentT, ChildT>() <= complex::get_RelationLevel<NextParentT, ChildT>();
		};

		template<typename ParentT, typename NextParentT, typename... Ts>
		struct	is_sorted<dpl::TypeList<ParentT, NextParentT, Ts...>>
		{
			static constexpr bool value = is_sorted<dpl::TypeList<ParentT, NextParentT>>::value && is_sorted<dpl::TypeList<NextParentT, Ts...>>::value;
		};

		template <ParentTypeList FILTERED_PARENT_TYPES, ParentTypeList UNFILTERED_PARENT_TYPES>
		struct	UniqueLevelFilter;

		template <typename... FilteredTs, typename... UnfilteredTs>
		struct	UniqueLevelFilter<dpl::TypeList<FilteredTs...>, dpl::TypeList<UnfilteredTs...>>
		{
			using Levels = dpl::TypeList<MyRelation<PairTag<FilteredTs, ChildT>::LEVEL>...>;
		};

		template <typename... FilteredTs, typename ParentT, typename... UnfilteredTs>
		struct	UniqueLevelFilter<dpl::TypeList<FilteredTs...>, dpl::TypeList<ParentT, UnfilteredTs...>>
				: std::conditional_t<(is_same_level<FilteredTs, ParentT>::value || ...)
				, UniqueLevelFilter<dpl::TypeList<FilteredTs...>, dpl::TypeList<UnfilteredTs...>>
				, UniqueLevelFilter<dpl::TypeList<FilteredTs..., ParentT>, dpl::TypeList<UnfilteredTs...>>> {};

		template<ParentTypeList PARENT_TYPES, typename ChildT, uint32_t LEVEL>
		class	PartialBase;

		template<typename Levels>
		class	AssembledBase; //<-- Splits child base into parts, one per relation level.

		template<typename ParentT, uint32_t LEVEL>
		using	InternalParentType = std::conditional_t<std::is_same_v<ParentT, UnknownParent>, RuntimeParent<ChildT, LEVEL>, ParentT>;

		template<typename ParentT, typename ChildT, uint32_t LEVEL> // Specialization for single parent type.
		class	PartialBase<dpl::TypeList<ParentT>, ChildT, LEVEL> : public IChild<InternalParentType<ParentT, LEVEL>, ChildT, complex::get_RelationType<ChildT, LEVEL>(), LEVEL>
		{
		public: // friends
			template<typename>
			friend class AssembledBase;

		private: // constants
			static constexpr bool bHAS_RUNTIME_PARENT = std::is_same_v<ParentT, UnknownParent>;

		private: // subtypes
			using MyParent	= InternalParentType<ParentT, LEVEL>;
			using MyBase	= IChild<MyParent, ChildT, complex::get_RelationType<ChildT, LEVEL>(), LEVEL>;
			using ParentRef = std::conditional_t<std::is_same_v<ParentT, UnknownParent>, const Origin<ChildT, LEVEL>&, ParentT&>;

		private: // lifecycle
			CLASS_CTOR					PartialBase(	ParentRef						parent)
				: MyBase(PartialBase::get_parentPtr(parent))
			{
			
			}

			CLASS_CTOR					PartialBase(	const PartialBase&				OTHER) = delete;
			CLASS_CTOR					PartialBase(	PartialBase&&					other) noexcept = default;

			inline PartialBase&			operator=(		const PartialBase&				OTHER) = delete;
			inline PartialBase&			operator=(		PartialBase&&					other) noexcept = default;

			inline PartialBase&			operator=(		dpl::Swap<PartialBase>			other) noexcept
			{
				MyBase::operator=(dpl::Swap<MyBase>(*other));
				return *this;
			}

		private: // functions
			inline MyParent*			get_parentPtr(	ParentT&						parent)
			{
				return &parent;
			}

			inline MyParent*			get_parentPtr(	const Origin<ChildT, LEVEL>&	PROXY)
			{
				return PROXY.parent;
			}

			template<typename ParentU>
			static constexpr uint32_t	get_parentTypeID()
			{
				return 0;
			}

			template<typename ParentU>
			inline bool					has_parent() const
			{
				return MyBase::has_parent() && std::is_same_v<ParentU, ParentT>;
			}

			template<typename ParentU>
			inline ParentU&				get_parent()
			{
				validate_parent<ParentU>();
				return MyBase::get_parent();
			}

			template<typename ParentU>
			inline const ParentU&		get_parent() const
			{
				validate_parent<ParentU>();
				return MyBase::get_parent();
			}

			template<typename ParentU>
			inline ChildT*				previous_sibling()
			{
				validate_parent<ParentU>();
				return MyBase::previous_sibling();
			}

			template<typename ParentU>
			inline const ChildT*		previous_sibling() const
			{
				validate_parent<ParentU>();
				return MyBase::previous_sibling();
			}

			template<typename ParentU>
			inline ChildT*				next_sibling()
			{
				validate_parent<ParentU>();
				return MyBase::next_sibling();
			}

			template<typename ParentU>
			inline const ChildT*		next_sibling() const
			{
				validate_parent<ParentU>();
				return MyBase::next_sibling();
			}

			template<typename ParentU>
			inline void					validate_parent() const
			{
				#ifdef _DEBUG
				if(has_parent<ParentU>()) return;
				throw dpl::GeneralException(this, __LINE__, "Invalid parent access.");
				#endif // _DEBUG
			}
		};

		template<typename... ParentTs, typename ChildT, uint32_t LEVEL> // Specialization for child that allows for parent selection.
		class	PartialBase<dpl::TypeList<ParentTs...>, ChildT, LEVEL> : public IChild<RuntimeParent<ChildT, LEVEL>, ChildT, complex::get_RelationType<ChildT, LEVEL>(), LEVEL> 
		{
		public: // friends
			template<typename>
			friend class AssembledBase;

		private: // subtypes
			using MyBase	= IChild<RuntimeParent<ChildT, LEVEL>, ChildT, complex::get_RelationType<ChildT, LEVEL>(), LEVEL>;
			using ParentRef = const Origin<ChildT, LEVEL>&;

		public: // data
			dpl::ReadOnly<uint32_t, PartialBase> parentTypeID; // Index of the parent type within the type list of the parents of this origin, or INVALID_ID in case parent was optional and unspecified.

		private: // lifecycle
			CLASS_CTOR					PartialBase(	ParentRef				proxy)
				: MyBase(proxy.parent)
				, parentTypeID(proxy.TYPE_ID)
			{
			
			}

			CLASS_CTOR					PartialBase(	const PartialBase&		OTHER) = delete;
			CLASS_CTOR					PartialBase(	PartialBase&&			other) noexcept = default;

			inline PartialBase&			operator=(		const PartialBase&		OTHER) = delete;
			inline PartialBase&			operator=(		PartialBase&&			other) noexcept = default;

			inline PartialBase&			operator=(		dpl::Swap<PartialBase>	other) noexcept
			{
				MyBase::operator=(dpl::Swap<MyBase>(*other));
				parentTypeID.swap(other->parentTypeID);
				return *this;
			}

		private: // functions
			template<typename ParentT>
			static constexpr uint32_t	get_parentTypeID()
			{
				return dpl::TypeList<ParentTs...>::template Type<ParentT>::INDEX;
			}

			template<typename ParentT>
			inline bool					has_parent() const
			{
				return MyBase::has_parent() && (get_parentTypeID<ParentT>() == parentTypeID());
			}

			template<typename ParentT>
			inline ParentT&				get_parent()
			{
				validate_parent<ParentT>();
				return static_cast<ParentT&>(MyBase::get_parent());
			}

			template<typename ParentT>
			inline const ParentT&		get_parent() const
			{
				validate_parent<ParentT>();
				return static_cast<const ParentT&>(MyBase::get_parent());
			}

			template<typename ParentT>
			inline ChildT*				previous_sibling()
			{
				validate_parent<ParentT>();
				return MyBase::previous_sibling();
			}

			template<typename ParentT>
			inline const ChildT*		previous_sibling() const
			{
				validate_parent<ParentT>();
				return MyBase::previous_sibling();
			}

			template<typename ParentT>
			inline ChildT*				next_sibling()
			{
				validate_parent<ParentT>();
				return MyBase::next_sibling();
			}

			template<typename ParentT>
			inline const ChildT*		next_sibling() const
			{
				validate_parent<ParentT>();
				return MyBase::next_sibling();
			}

			template<typename ParentT>
			inline void					validate_parent() const
			{
				#ifdef _DEBUG
				if(has_parent<ParentT>()) return;
				throw dpl::GeneralException(this, __LINE__, "Invalid parent access.");
				#endif // _DEBUG
			}
		};

		template<> // Specialization for parentless child.
		class	AssembledBase<dpl::TypeList<>>
		{
		public: // subtypes
			template<typename LevelT>
			using ParentRef = int; //<-- Solves "Failed alias template". Note: This type is never actually used, it just have to be anything, but void.
		};

		template<typename... LevelTs>
		class	AssembledBase<dpl::TypeList<LevelTs...>> : public PartialBase<typename LevelTs::Parents, ChildT, LevelTs::LEVEL>...
		{
		public: // friends
			template<ParentTypeList, typename, uint32_t>
			friend class PartialBase;

			template<typename, typename, RelationType, uint32_t>
			friend class IChild;

			template<typename, typename, RelationType, uint32_t>
			friend class IParent;

		public: // subtypes
			template<uint32_t LEVEL>
			using MyLevel	= MyRelation<LEVEL>;

		private: // subtypes
			template<typename LevelT>
			using MyBase_t	= PartialBase<typename LevelT::Parents, ChildT, LevelT::LEVEL>;

			template<uint32_t LEVEL>
			using MyBase_i	= PartialBase<typename MyRelation<LEVEL>::Parents, ChildT, LEVEL>;

			template<typename ParentT>
			using MyBase_o	= MyBase_i<complex::get_RelationLevel<ParentT, ChildT>()>;

		public: // subtypes
			template<typename LevelT>
			using ParentRef = typename MyBase_t<LevelT>::ParentRef;

		public: // lifecycle
			CLASS_CTOR					AssembledBase(	ParentRef<LevelTs>...		parent)
				: MyBase_t<LevelTs>(parent)...
			{

			}

		protected: // lifecycle
			CLASS_CTOR					AssembledBase(	const AssembledBase&		OTHER) = delete;
			CLASS_CTOR					AssembledBase(	AssembledBase&&				other) noexcept = default;

			inline AssembledBase&		operator=(		const AssembledBase&		OTHER) = delete;
			inline AssembledBase&		operator=(		AssembledBase&&				other) noexcept = default;

			inline AssembledBase&		operator=(		dpl::Swap<AssembledBase>	other) noexcept
			{
				(MyBase_t<LevelTs>::operator=(dpl::Swap<MyBase<LevelTs>>(*other)), ...);
				return *this;
			}

		public: // functions
			template<typename ParentT>
			static constexpr bool		has_parent_type()
			{
				return complex::has_relation<ParentT, ChildT>();
			}

			static constexpr bool		has_unknown_parent_type()
			{
				return AssembledBase::has_parent_type<complex::UnknownParent>();
			}

			template<typename ParentT>
			static constexpr bool		is_parent_type_unknown()
			{
				return !AssembledBase::has_parent_type<ParentT>() && AssembledBase::has_unknown_parent_type();
			}

			template<typename ParentT>
			static constexpr bool		has_runtime_parent()
			{
				if constexpr (AssembledBase::is_parent_type_unknown<ParentT>()) return true;
				else return MyRelation<complex::get_RelationLevel<ParentT, ChildT>()>::Parents::SIZE > 1;
			}

			template<uint32_t LEVEL>
			static constexpr bool		is_primary_level()
			{
				return dpl::TypeList<LevelTs...>::template Type<MyRelation<LEVEL>>::INDEX == 0; 
			}

			template<typename ParentT>
			static constexpr uint32_t	get_parentTypeID()
			{
				if constexpr (AssembledBase::is_parent_type_unknown<ParentT>()) return MyBase_o<UnknownParent>::template get_parentTypeID<UnknownParent>();
				else return MyBase_o<ParentT>::template get_parentTypeID<ParentT>();
			}

			template<typename ParentT>
			inline bool					has_parent() const
			{
				return MyBase_o<ParentT>::template has_parent<ParentT>();
			}

			template<typename ParentT>
			inline const ParentT&		get_parent() const
			{
				return MyBase_o<ParentT>::template get_parent<ParentT>();
			}

			template<typename ParentT>
			inline const ChildT*		previous_sibling() const
			{
				return MyBase_o<ParentT>::template previous_sibling<ParentT>();
			}

			template<typename ParentT>
			inline const ChildT*		next_sibling() const
			{
				return MyBase_o<ParentT>::template next_sibling<ParentT>();
			}
		};

	public: // subtypes
		static_assert(is_sorted<PARENT_TYPES>::value, "Parent type list must be sorted by relation level.");

		using Levels	= UniqueLevelFilter<dpl::TypeList<>, PARENT_TYPES>::Levels;
		using Type		= AssembledBase<Levels>;

	private: // lifecycle
		Child() = delete;
		~Child() = delete;
	};
}

// parent implementation
namespace complex
{
	template <typename ParentT, ChildTypeList CHILD_TYPES>
	class	ParentWithManyChildren
	{
	public: // friends
		template<ParentTypeList, typename>
		friend class Child;

	private: // subtypes
		template<typename ChildT>
		static constexpr bool			is_unknown()
		{
			return ChildT::template is_parent_type_unknown<ParentT>();
		}

		template<typename ChildT>
		static constexpr uint32_t		get_level()
		{
			if constexpr (ParentWithManyChildren::is_unknown<ChildT>()) return complex::get_RelationLevel<UnknownParent, ChildT>();
			else return complex::get_RelationLevel<ParentT, ChildT>();
		}

		template<typename ChildT>
		static constexpr RelationType	get_type()
		{
			return complex::get_RelationType<ChildT, ParentWithManyChildren::get_level<ChildT>()>();
		}

		template<typename ChildT>
		struct	Partial
		{
			static constexpr uint32_t		LEVEL			= ParentWithManyChildren::get_level<ChildT>();
			static constexpr RelationType	TYPE			= ParentWithManyChildren::get_type<ChildT>();
			static constexpr bool			bRUNTIME_PARENT = ChildT::template has_runtime_parent<ParentT>();
			static constexpr bool			bPRIMARY_PARENT = ChildT::template is_primary_level<LEVEL>();

			using Base = std::conditional_t<bRUNTIME_PARENT, RuntimeParent<ChildT, LEVEL>, IParent<ParentT, ChildT, TYPE, LEVEL> >;
		};
		
		template<typename ChildT>
		using	PartialBase = Partial<ChildT>::Base;

		template<ChildTypeList CHILD_TYPES_X>
		class	AssembledBase; //<-- Splits parent base into parts, one per child type.

		template<>
		class	AssembledBase<dpl::TypeList<>> // Specialization for childless parent.
		{
		public: // subtypes
			using	Dependency	= dpl::TypeList<>;
		};

		template<typename... ChildTs>
		class	AssembledBase<dpl::TypeList<ChildTs...>> : private PartialBase<ChildTs>...
		{
		public: // friends
			template<ParentTypeList, typename>
			friend class Child;

			template<typename, uint32_t, bool>
			friend class OriginBase;

			template<typename, typename, uint32_t>
			friend class dpl::Association;

		public: // subtypes
			template <typename ChildT>
			using	Invoke		= std::function<void(ChildT&)>;

			template <typename ChildT>
			using	InvokeConst	= std::function<void(const ChildT&)>;

			using	Dependency	= dpl::merge_t<dpl::TypeList<ChildTs...>, typename ChildTs::Dependency...>;

		protected: // lifecycle
			CLASS_CTOR					AssembledBase() = default;
			CLASS_CTOR					AssembledBase(	const AssembledBase&		OTHER) = delete;
			CLASS_CTOR					AssembledBase(	AssembledBase&&				other) noexcept = default;

			inline AssembledBase&		operator=(		const AssembledBase&		OTHER) = delete;
			inline AssembledBase&		operator=(		AssembledBase&&				other) noexcept = default;

			inline AssembledBase&		operator=(		dpl::Swap<AssembledBase>	other) noexcept
			{
				(PartialBase<ChildTs>::operator=(dpl::Swap<PartialBase<ChildTs>>(*other)), ...);
				return *this;
			}

		public: // functions
			template<typename ChildT>
			static constexpr bool		is_primary_parent_of()
			{
				constexpr bool bHAS_CHILD_TYPE = dpl::TypeList<ChildTs...>::template has_type<ChildT>();
				static_assert(bHAS_CHILD_TYPE, "Unrelated child");
				if constexpr (bHAS_CHILD_TYPE)	return Partial<ChildT>::bPRIMARY_PARENT;
				else							return false;
			}

			template<typename ChildT>
			inline uint32_t				get_numChildren() const
			{
				return PartialBase<ChildT>::get_numChildren();
			}

			template<typename ChildT>
			inline bool					can_have_another_child() const
			{
				return PartialBase<ChildT>::can_have_another_child();
			}

			template<typename ChildT>
			inline bool					has_children() const
			{
				return AssembledBase::get_numChildren<ChildT>() > 0;
			}

			template<typename ChildT>
			inline ChildT*				get_first_child()
			{
				return PartialBase<ChildT>::get_first_child();
			}

			template<typename ChildT>
			inline const ChildT*		get_first_child() const
			{
				return PartialBase<ChildT>::get_first_child();
			}

			template<typename ChildT>
			inline void					for_each_child(	const Invoke<ChildT>&		FUNCTION)
			{
				PartialBase<ChildT>::for_each_child(FUNCTION);
			}

			template<typename ChildT>
			inline void					for_each_child(	const InvokeConst<ChildT>&	FUNCTION) const
			{
				PartialBase<ChildT>::for_each_child(FUNCTION);
			}

		protected: // functions
			template<typename ChildT, typename ComparatorT>
			inline void					sort_children(	ComparatorT&&				compare)
			{
				PartialBase<ChildT>::sort(std::forward<ComparatorT>(compare));
			}

		private: // functions
			template<typename ChildT>
			inline bool					remove_child(	ChildT&						child)
			{
				return PartialBase<ChildT>::remove_child(child);
			}
		};

	public: // subtypes
		using Type	= AssembledBase<CHILD_TYPES>;
	};
}
#pragma pack(pop)