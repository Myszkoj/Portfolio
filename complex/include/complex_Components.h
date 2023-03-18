#pragma once


#include <tuple>
#include <dpl_NamedType.h>
#include <dpl_TypeTraits.h>
#include <dpl_DataTransfer.h>
#include "complex_Utilities.h"


#pragma pack(push, 4)

// requirements
namespace complex
{
	template<typename ComponentT>
	concept is_Component			= std::is_default_constructible_v<ComponentT>
									&& std::is_copy_assignable_v<ComponentT>
									&& is_divisible_by<ComponentT, 4>;

	template<typename ComponentT>
	struct	IsComponent
	{
		static const bool value = is_Component<ComponentT>;
	};

	template<typename COMPONENT_TYPES>
	concept	is_ComponentTypeList	= dpl::is_TypeList<COMPONENT_TYPES> 
									&& COMPONENT_TYPES::ALL_UNIQUE 
									&& COMPONENT_TYPES::template all<IsComponent>();
}

// component table implementation
namespace complex
{
	/*
		Stores NxM components, where N is the number of component types(columns) and M is the number composites(rows).
	*/
	template<typename CompositeT, is_ComponentTypeList COMPONENT_TYPES>
	class	ComponentTable;

	
	template<typename CompositeT, typename... ComponentTn>
	class	ComponentTable<CompositeT, dpl::TypeList<ComponentTn...>>
	{
	public: // subtypes
		using	COMPONENT_TYPES = dpl::TypeList<ComponentTn...>;
		using	Column			= std::tuple<ComponentTn*...>;
		using	ConstColumn		= std::tuple<const ComponentTn*...>;

		template<is_Component T>
		using	ColumnStorage	= std::conditional_t<	std::is_trivially_destructible_v<T>,
														dpl::TransferablePack<T>, 
														dpl::DynamicArray<T>>;

		template<is_Component T>
		class	Row	: private ColumnStorage<T>
		{
		private: // subtypes
			using	MyStorageBase	= ColumnStorage<T>;

		public: // subtypes
			using	Invocation		= typename ColumnStorage<T>::Invocation;
			using	ConstInvocation	= typename ColumnStorage<T>::ConstInvocation;

		public: // constants
			static constexpr bool IS_TRANSFERABLE = std::is_same_v<MyStorageBase, dpl::TransferablePack<T>>;

		public: // friends
			friend	MyStorageBase;
			friend	ComponentTable;

		public: // exposed functions
			using	MyStorageBase::size;
			using	MyStorageBase::index_of;

		public: // lifecycle
			CLASS_CTOR			Row() = default;
			CLASS_CTOR			Row(				Row&&					other) noexcept = default;
			Row&				operator=(			Row&&					other) noexcept = default;

		private: // lifecycle (deleted)
			CLASS_CTOR			Row(				const Row&				OTHER) = delete;
			Row&				operator=(			const Row&				OTHER) = delete;

		public: // functions
			inline uint32_t		offset() const
			{
				if constexpr(IS_TRANSFERABLE)	return MyStorageBase::offset;
				else							return 0;
			}

			inline T*			modify()
			{
				if constexpr(IS_TRANSFERABLE)	return MyStorageBase::modify();
				else							return MyStorageBase::data();
			}

			inline const T*		read() const
			{
				if constexpr(IS_TRANSFERABLE)	return MyStorageBase::read();
				else							return MyStorageBase::data();
			}

			inline void			modify_each(		const Invocation&		INVOKE)
			{
				if constexpr(IS_TRANSFERABLE)	return MyStorageBase::modify_each(INVOKE);
				else							return MyStorageBase::for_each(INVOKE);
			}

			inline void			read_each(			const ConstInvocation&	INVOKE) const
			{
				if constexpr(IS_TRANSFERABLE)	return MyStorageBase::read_each(INVOKE);
				else							return MyStorageBase::for_each(INVOKE);
			}

			inline T*			at(					const uint32_t			COLUMN_INDEX)
			{
				return modify() + COLUMN_INDEX;
			}

			inline const T*		at(					const uint32_t			COLUMN_INDEX) const
			{
				return read() + COLUMN_INDEX;
			}

			inline uint32_t		index_of(			const T*				COMPONENT_ADDRESS) const
			{
				return MyStorageBase::index_of(COMPONENT_ADDRESS);
			}

		private: // internal functions
			inline T*			enlarge(			const uint32_t			NUM_COLUMNS)
			{
				return MyStorageBase::enlarge(NUM_COLUMNS);
			}

			inline void			destroy_at(			const uint32_t			COLUMN_INDEX)
			{
				MyStorageBase::fast_erase(COLUMN_INDEX);
			}
		};

		using	Rows			= std::tuple<Row<ComponentTn>...>;

	private: // data
		Rows m_rows; //<-- Each row has the same size.

	protected: // lifecycle
		CLASS_CTOR						ComponentTable() = default;

	public: // row functions
		static constexpr uint32_t		numRows()
		{
			return COMPONENT_TYPES::SIZE;
		}

		template<dpl::is_one_of<COMPONENT_TYPES> T>
		inline Row<T>&					row()
		{
			return std::get<Row<T>>(m_rows);
		}

		template<dpl::is_one_of<COMPONENT_TYPES> T>
		inline const Row<T>&			row() const
		{
			return std::get<Row<T>>(m_rows);
		}

	public: // column functions
		inline uint32_t					numColumns() const
		{
			using FirstComponentType = COMPONENT_TYPES::template At<0>;
			return Row<FirstComponentType>::size();
		}

		inline Column					column(			const uint32_t		COLUMN_INDEX)
		{
			return std::make_tuple(ComponentTable::row<ComponentTn>().at(COLUMN_INDEX)...);
		}

		inline ConstColumn				column(			const uint32_t		COLUMN_INDEX) const
		{
			return std::make_tuple(ComponentTable::row<ComponentTn>().at(COLUMN_INDEX)...);
		}

		template<dpl::is_one_of<COMPONENT_TYPES> T>
		inline uint32_t					column_index(	const T*			COMPONENT_ADDRESS) const
		{
			return ComponentTable::row<T>().index_of(COMPONENT_ADDRESS);
		}

	protected: // column functions
		inline Column					add_columns(	const uint32_t		NUM_COLUMNS)
		{
			return std::make_tuple(ComponentTable::row<ComponentTn>().enlarge(NUM_COLUMNS)...);
		}

		inline Column					add_column()
		{
			return ComponentTable::add_columns(1);
		}
			
		inline void						remove_column(	const uint32_t		COLUMN_INDEX)
		{
			(ComponentTable::row<ComponentTn>().destroy_at(COLUMN_INDEX), ...);
		}
	};
}

#pragma pack(pop)