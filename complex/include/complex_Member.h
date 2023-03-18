#pragma once


#include <dpl_NamedType.h>
#include <dpl_Ownership.h>
#include <dpl_Mask.h>
#include <dpl_Binary.h>
#include <dpl_GeneralException.h>
#include "complex_Relations.h"

#pragma pack(push, 4)
// flags
namespace complex
{
	enum	Flags : uint32_t
	{
		// BIT INDICES
		INHERITED_INSTANCES_INDEX			= 0,
		DISABLE_UNDO_REDU_INSTANCES_INDEX	= 1,

		// BIT MASKS
		NONE						= 0,
		INHERITED_INSTANCES			= (1<<INHERITED_INSTANCES_INDEX),			// Number of instances is controlled by the parent object.
		DISABLE_UNDO_REDU_INSTANCES	= (1<<DISABLE_UNDO_REDU_INSTANCES_INDEX),	// Create/invalidate operations are irreversible for instances. Mutually exclusive with INHERITED_INSTANCES.
		SETTINGS_MASK				= INHERITED_INSTANCES | DISABLE_UNDO_REDU_INSTANCES
	};

	enum	InternalFlags : uint32_t
	{
		// BIT INDICES
		LOCKED_INDEX				= 2,// Object cannot be destroyed, or spawn any children and instances.
		MARKED_AS_INVALID_INDEX		= 3,// Object/Instance is no longer valid and will be destroyed on next World::clear_commands.
		PARENT_INVALIDATED_INDEX	= 4,

		// All flags above are reserved for internal usage.
		FIRST_USER_FLAG_INDEX		= 10,

		// BIT MASKS
		LOCKED					= (1<<LOCKED_INDEX),
		MARKED_AS_INVALID		= (1<<MARKED_AS_INVALID_INDEX),
		PARENT_INVALIDATED		= (1<<PARENT_INVALIDATED_INDEX),
		INVALID = MARKED_AS_INVALID | PARENT_INVALIDATED
	};

	const	dpl::IndexRange<uint32_t>	USER_FLAG_RANGE(FIRST_USER_FLAG_INDEX, 31);
	const	dpl::Mask32_t				USER_FLAGS_MASK(USER_FLAG_RANGE);


	class	Entity
	{
	public: // data
		dpl::ReadOnly<dpl::Mask32_t, Entity> flags; // [INTERNAL_FLAGS][USER_FLAGS]

	public: // lifecycle
		CLASS_CTOR		Entity(				const dpl::Mask32<Flags>	FLAGS)
			: flags(FLAGS.get() & Flags::SETTINGS_MASK)
		{
								
		}

	public: // operators
		Entity&			operator=(			dpl::Swap<Entity>			other)
		{
			flags.swap(other->flags);
			return *this;
		}

	public: // functions
		inline bool		is_valid() const
		{
			return !flags().any_of(InternalFlags::INVALID);
		}

		inline bool		flag_at(			const uint32_t				FLAG_INDEX) const
		{
			return flags().at(FLAG_INDEX);
		}

		inline bool		flags_fit(			const uint32_t				MASK) const
		{
			return flags().fits(MASK);
		}

	protected: // functions
		inline void		set_user_flag(		const uint32_t				FLAG_INDEX,
											const bool					bSTATE)
		{
			if(FLAG_INDEX < FIRST_USER_FLAG_INDEX)
				throw dpl::GeneralException(this, __LINE__, "User flags start at: " + std::to_string(FLAG_INDEX));

			flags->set_at(FLAG_INDEX, bSTATE);
		}

		inline void		set_user_flags(		const uint32_t				USER_FLAGS)
		{
			*flags | (USER_FLAGS & USER_FLAGS_MASK);
		}

	protected: // functions
		inline void		set_flag(			const uint32_t				FLAG_INDEX,
											const bool					bSTATE)
		{
			flags->set_at(FLAG_INDEX, bSTATE);
		}
	};
}

// declarations
namespace complex
{
	template<typename ObjT>
	class	ObjectGroup;

	/*
		Handles parent-child relations between the objects.
	*/
	template<typename ObjT, ParentTypeList PARENT_TYPES, ChildTypeList CHILD_TYPES, LevelTypesList LEVEL_TYPES>
	class	RelationNode;
}

// definitions
namespace complex
{
	/*
		Defines object as a member of the group.
		Object is owned by the group and stored in a contiguoes array with unique index assigned to it.
		Each object is also uniquely named and described by 32bit mask of flags.
		Flags are divided into internal flags and user flags, that start at FIRST_USER_FLAG_INDEX.
	*/
	template<typename ObjT>
	class	Member	: public dpl::NamedType<ObjT>
					, private dpl::Property<ObjectGroup<ObjT>, ObjT>
					, public Entity
	{
	private: // subtypes
		using	MyTypeNameBase	= dpl::NamedType<ObjT>;
		using	MyGroup			= ObjectGroup<ObjT>;
		using	MyPropertyBase	= dpl::Property<MyGroup, ObjT>;
		using	MyOwnerType		= dpl::DynamicOwner<MyGroup, ObjT>;

	public: // friends
		friend	MyPropertyBase;
		friend	MyOwnerType;
		friend	ObjectGroup<ObjT>;

	public: // exposed functions
		using	MyPropertyBase::INVALID_INDEX;
		using	MyPropertyBase::get_index;

	protected: // lifecycle
		CLASS_CTOR				Member(			const dpl::Ownership&		OWNERSHIP,
												const dpl::Mask32<Flags>	FLAGS)
			: MyPropertyBase(OWNERSHIP)
			, Entity(FLAGS)
		{
								
		}

		CLASS_CTOR				Member(			const Member&				OTHER) = delete;

		CLASS_CTOR				Member(			const Ownership&			OWNERSHIP,
												Member&&					other) noexcept
			: MyPropertyBase(OWNERSHIP, std::move(other))
			, Entity(other)
		{

		}

		Member&					operator=(		const Member&				OTHER) = delete;

		Member&					operator=(		Member&&					other) noexcept = delete;

		Member&					operator=(		dpl::Swap<Member>			other)
		{
			Entity::operator=(dpl::Swap<Entity>(other));
			return *this;
		}

	public: // functions
		static inline uint32_t	get_groupID()
		{
			return ObjectGroup<ObjT>::get_typeID();
		}
	};


	template<typename ObjT, ParentTypeList _PARENT_TYPES_, typename... ChildTs, typename... LevelTs>
	class	RelationNode<ObjT, _PARENT_TYPES_, ChildTypes<ChildTs...>, LevelTypes<LevelTs...>>
		: private Member<ObjT>
		, public Child<_PARENT_TYPES_, ObjT>::Type
		, public ParentWithManyChildren<ObjT, ChildTypes<ChildTs...>>::Type
	{
	public: // friends
		template<typename, ParentTypeList, ChildTypeList, LevelTypesList>
		friend class RelationNode;

		friend	ObjectGroup<ObjT>;

	public: // subtypes
		using	PARENT_TYPES	= _PARENT_TYPES_;
		using	CHILD_TYPES		= ChildTypes<ChildTs...>;
		using	LEVEL_TYPES		= LevelTypes<LevelTs...>;

	private: // subtypes
		using	MyMemberBase	= Member<ObjT>;
		using	MyChildBase		= typename Child<_PARENT_TYPES_, ObjT>::Type;
		using	MyParentBase	= typename ParentWithManyChildren<ObjT, CHILD_TYPES>::Type;

	public: // constants
		static constexpr bool ROOT	= _PARENT_TYPES_::SIZE == 0;

	public: // subtypes
		template<typename LevelT>
		using	ParentRef			= MyChildBase::template ParentRef<LevelT>;

		using	Dependency			= std::conditional_t<ROOT , dpl::merge_t<dpl::TypeList<ObjT>, MyParentBase::Dependency>, MyParentBase::Dependency>::Unique::Type;

	public: // commands
		/*
			Creates object indirectly making it a reversable operation.
			Undoing object creation does not cause immediate destruction,
			instead it is marked as invalid and will be destroyed during next Project::clear_commands call.
		*/
		class	CreateObject : public dpl::Command
		{
		public: // data
			dpl::ReadOnly<uint32_t, CreateObject> objIndex;

		public: // lifecycle
			template<typename... UserArgs>
			CLASS_CTOR		CreateObject(	ParentRef<LevelTs>...	parent,
											UserArgs&&...			args)
				: objIndex(RelationNode::spawn_indirectly(parent..., std::forward<UserArgs>(args)...).get_index())
			{
				
			}

		private: // implementation
			virtual void	execute()
			{
				ObjectGroup<ObjT>::ref().get_object(objIndex()).set_INVALIDATED(false);
			}

			virtual void	unexecute()
			{
				ObjectGroup<ObjT>::ref().get_object(objIndex()).set_INVALIDATED(true);
			}
		};

		class	CreateObjects : public dpl::Command
		{
		private: // data
			std::vector<CreateObject> commands;

		public: // functions
			inline void		reserve(	const uint32_t			NUM_OBJECTS_TO_CREATE)
			{
				commands.reserve(NUM_OBJECTS_TO_CREATE);
			}

			template<typename... UserArgs>
			inline ObjT&	create(		ParentRef<LevelTs>...	parent,
										UserArgs&&...			args)
			{
				CreateObject& newCommand = commands.emplace_back(std::forward<ParentRef<LevelTs>>(parent)..., std::forward<UserArgs>(args)...);
				return ObjectGroup<ObjT>::ref().get_object_at(newCommand.objIndex());
			}

		private: // implementation
			virtual bool	valid() const final override
			{
				return !commands.empty();
			}

			virtual void	execute() final override
			{
				for(dpl::Command& command : commands) command.execute();
			}

			virtual void	unexecute() final override
			{
				for(dpl::Command& command : commands) command.unexecute();
			}
		};

		/*
			This command invalidates object and all of its children.
			Objects and instances are destroyed during Project::clear_commands call.
		*/
		class	InvalidateObject : public dpl::Command
		{
		private: // data
			dpl::ReadOnly<uint32_t, InvalidateObject> index;

		public: // lifecycle
			CLASS_CTOR		InvalidateObject(const Member<ObjT>& OBJECT)
				: index(OBJECT.get_index())
			{

			}

		public: // implementation
			virtual bool	valid() const final override
			{
				if(index() >= ObjectGroup<ObjT>::ref().size()) return false;
				const Member<ObjT>& OBJECT = ObjectGroup<ObjT>::ref().get_object(index());
				return OBJECT.is_valid();
			}

			virtual void	execute() final override
			{
				ObjectGroup<ObjT>::ref().get_object(index()).set_invalidated(true);
				// TODO: send event
			}

			virtual void	unexecute() final override
			{
				ObjectGroup<ObjT>::ref().get_object(index()).set_invalidated(false);
				// TODO: send event
			}
		};

		class	InvalidateObjects : public dpl::Command
		{
		private: // data
			std::vector<InvalidateObject> commands;

		public: // functions
			/*
				Reserves space for per-object commands.
			*/
			inline void		reserve(				const uint32_t		NUM_OBJECTS_TO_INVALIDATE)
			{
				commands.reserve(NUM_OBJECTS_TO_INVALIDATE);
			}

			/*
				Adds object to invalidate.
				Ignoresobjects that are already invalid.
				Although it is not recommended, it is possible to specify the same object more than once without any problems.
			*/
			inline void		invalidate(				const Member<ObjT>&	OBJECT)
			{
				if(!OBJECT.is_valid()) return;
				commands.emplace_back(OBJECT);
			}

			template<typename ParentT>
			inline void		invalidate_children_of(	const ParentT&		PARENT)
			{
				const size_t NUM_COMMANDS = commands.size() + PARENT.get_numChildren<ObjT>();
				if(commands.capacity() < NUM_COMMANDS) commands.reserve(NUM_COMMANDS);	
				PARENT.for_each_child<ObjT>([&](const Member<ObjT>& CHILD)
				{
					invalidate(CHILD);
				});
			}

		private: // implementation
			virtual bool	valid() const final override
			{
				return !commands.empty();
			}

			virtual void	execute() final override
			{
				for(dpl::Command& command : commands) command.execute();
			}

			virtual void	unexecute() final override
			{
				for(dpl::Command& command : commands) command.unexecute();
			}
		};

	public: // inherited functions
		using	MyMemberBase::INVALID_INDEX;
		using	MyMemberBase::get_index;
		using	MyMemberBase::is_valid;
		using	MyMemberBase::flag_at;
		using	MyMemberBase::flags_fit;

	protected: // inherited functions
		using	MyMemberBase::set_user_flag;
		using	MyMemberBase::set_user_flags;

	protected: // lifecycle
		CLASS_CTOR			RelationNode(			const dpl::Ownership&		OWNERSHIP,
													const dpl::Mask32<Flags>	FLAGS,
													ParentRef<LevelTs>...		parent)
			: MyMemberBase(OWNERSHIP, FLAGS)
			, MyChildBase(parent...)
		{
			
		}

		CLASS_CTOR			RelationNode(			const dpl::Ownership&		OWNERSHIP,
													RelationNode&&				other) noexcept
			: MyMemberBase(OWNERSHIP, std::move(other))
			, MyChildBase(std::move(other))
			, MyParentBase(std::move(other))
		{

		}

		CLASS_CTOR			RelationNode(			const RelationNode&			OTHER) = delete;
		CLASS_CTOR			RelationNode(			RelationNode&&				other) noexcept = delete;

		RelationNode&		operator=(				const RelationNode&			OTHER) = delete;
		RelationNode&		operator=(				RelationNode&&				other) noexcept = delete;

		RelationNode&		operator=(				dpl::Swap<RelationNode>		other) noexcept
		{
			MyMemberBase::operator=(dpl::Swap<MyMemberBase>(*other));
			MyChildBase::operator=(dpl::Swap<MyChildBase>(*other));
			MyParentBase::operator=(dpl::Swap<MyParentBase>(*other));
			return *this;
		}

	private: // functions
		template<typename... UserArgs>
		static inline ObjT&	spawn_directly(			ParentRef<LevelTs>...		parent,
													UserArgs&&...				args)
		{
			return ObjectGroup<ObjT>::ref().create_object_directly(parent..., std::forward<UserArgs>(args)...);
		}

		template<typename... UserArgs>
		static inline ObjT&	spawn_indirectly(		ParentRef<LevelTs>...		parent,
													UserArgs&&...				args)
		{
			return ObjectGroup<ObjT>::ref().create_object_indirectly(parent..., std::forward<UserArgs>(args)...);
		}

		inline void			notify_invalidated(		const bool					bVALUE)
		{
			(MyParentBase::template for_each_child<ChildTs>([&](ChildTs& child)
			{
				child.access_flags().set(InternalFlags::PARENT_INVALIDATED, bVALUE);
				child.notify_invalidated(bVALUE);
			}), ...);
		}

		inline void			set_invalidated(		const bool					bVALUE)
		{
			MyMemberBase::access_flags().set(InternalFlags::MARKED_AS_INVALID, bVALUE);
			RelationNode::notify_invalidated(bVALUE);
		}
	};
}
#pragma pack(pop)