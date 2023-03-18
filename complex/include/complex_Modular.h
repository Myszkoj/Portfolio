#pragma once


#include <fstream>
#include <dpl_Variation.h>
#include <dpl_Binary.h>
#include "complex_Utilities.h"


namespace complex
{
	template<typename ModularT>
	class	Modular;


	template<typename ModularT>
	class	IModule	: private dpl::Variant<Modular<ModularT>, IModule<ModularT>>
	{
	private: // subtypes
		using	MyType			= IModule<ModularT>;
		using	MyModular		= Modular<ModularT>;
		using	MyVariation		= dpl::Variation<MyModular, MyType>;
		using	MyVariantBase	= dpl::Variant<MyModular, MyType>;

	public: // subtypes
		using	Binding			= typename MyVariantBase::Binding;
		using	VCTOR_Base		= typename MyVariantBase::VCTOR_Base;

	public: // relations
		friend	MyVariantBase;
		friend	MyModular;
		friend	MyVariation;

	protected: // lifecycle
		CLASS_CTOR					IModule(		const Binding&		BINDING)
			: MyVariantBase(BINDING)
		{
			
		}

	public: // access owner
		inline ModularT&			get_owner()
		{
			return static_cast<ModularT&>(MyVariantBase::get_variation());
		}

		inline const ModularT&		get_owner() const
		{
			return static_cast<const ModularT&>(MyVariantBase::get_variation());
		}

	public: // interface
		virtual const std::string&	get_name() const = 0;

	private: // interface
		/*
			Returns true if component processed event.
		*/
		virtual bool				on_event(		const dpl::Event&	EVENT)
		{
			return false;
		}

		virtual void				on_destroy(){}
	};


	template<typename ModularT, typename ModuleT>
	class	Module	: public IModule<ModularT>
	{
	private: // subtypes
		using	MyBase		= IModule<ModularT>;
		using	VCTOR_Base	= typename MyBase::VCTOR_Base;

	public: // subtypes
		using Binding = typename MyBase::Binding;

	protected: // lifecycle
		CLASS_CTOR					Module(		const Binding&		BINDING)
			: MyBase(BINDING)
		{
			
		}

		CLASS_CTOR					Module(		const Module&		OTHER) = delete;

		CLASS_CTOR					Module(		Module&&			other) noexcept = delete;

		Module&						operator=(	const Module&		OTHER) = delete;

		Module&						operator=(	Module&&			other) noexcept = delete;

	public: // functions
		static inline uint32_t		get_typeID()
		{
			return Modular<ModularT>::template get_typeID<ModuleT>();
		}

	public: // interface
		virtual const std::string&	get_name() const final override
		{
			return VCTOR_Base::template get_typeName<ModuleT>();
		}
	};


	template<typename ModularT>
	class	Modular : private dpl::Variation<Modular<ModularT>, IModule<ModularT>>
	{
	private: // subtypes
		using MyType			= Modular<ModularT>;
		using MyModule			= IModule<ModularT>;
		using MyVariant			= dpl::Variant<MyType, MyModule>;
		using MyModules			= dpl::Variation<MyType, MyModule>;

	public: // subtypes
		using UseModule			= std::function<void(MyModule&)>;
		using UseConstModule	= std::function<void(const MyModule&)>;

	public: // relations
		friend MyModules;
		friend MyVariant;
		friend MyVariant::VCTOR_Base;

	protected: // lifecycle
		CLASS_CTOR						Modular() = default;

	public: // module creation functions
		template<typename ModuleT, typename... CTOR>
		inline dpl::Result<ModuleT*>	create_module(			CTOR&&...				args)
		{
			static_assert(std::is_base_of_v<IModule<ModularT>, ModuleT>, "ModuleT must be derived from Module<ModularT, ModuleT>.");
			return MyModules::template create_variant<ModuleT>(std::forward<CTOR>(args)...);
		}

		template<typename ModuleT>
		inline void						create_modules()
		{
			Modular::create_module<ModuleT>();
		}

		template<typename M1, typename M2, typename... Mn>
		inline void						create_modules()
		{
			Modular::create_modules<M1>();
			Modular::create_modules<M2, Mn...>();
		}

		template<typename ModuleT>
		inline void						destroy_module()
		{
			if(MyModule* myModule = Modular::find_module<ModuleT>())
			{
				myModule->on_destroy();
				myModule->selfdestruct();
			}
		}

		inline void						destroy_all_modules()
		{
			MyModules::destroy_all_variants();
		}

	public: // module iteration
		inline void						for_each_module(		const UseModule&		FUNCTION)
		{
			MyModules::for_each_variant(FUNCTION);
		}

		inline void						for_each_module(		const UseConstModule&	FUNCTION) const
		{
			MyModules::for_each_variant(FUNCTION);
		}

	public: // utility functions
		inline uint32_t					get_numModules() const
		{
			return MyModules::get_numVariants();
		}

		inline uint32_t					pass_event_to_modules(	const dpl::Event&		EVENT)
		{
			uint32_t numReceived = 0;
			Modular::for_each_module([&](MyModule& module)
			{
				if(module.on_event(EVENT))
				{
					++numReceived;
				}
			});
			return numReceived;
		}

	public: // module access functions
		template<typename ModuleT>
		inline ModuleT*					find_module()
		{
			return MyModules::template find_variant<ModuleT>();
		}

		template<typename ModuleT>
		inline const ModuleT*			find_module() const
		{
			return MyModules::template find_variant<ModuleT>();
		}

		template<typename ModuleT>
		inline ModuleT&					get_module()
		{
			return MyModules::template get_variant<ModuleT>();
		}

		template<typename ModuleT>
		inline const ModuleT&			get_module() const
		{
			return MyModules::template get_variant<ModuleT>();
		}
	};
}