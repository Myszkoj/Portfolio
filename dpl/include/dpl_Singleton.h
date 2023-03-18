#pragma once


#include "dpl_GeneralException.h"
#include <typeinfo>
#include <mutex>


namespace dpl
{
	/*
		TODO: 'safe_*' functions should return handle with std::lock_guard and pointer to instance.
	*/
	template<class T>
	class Singleton
	{
	public: // functions
		static inline T*	ptr()
		{
			return static_cast<T*>(sm_instance);
		}

		static inline T*	get()
		{
			return static_cast<T*>(sm_instance);
		}

		static inline T*	safe_get()
		{
			std::lock_guard glock(sm_mutex);
			return static_cast<T*>(sm_instance);
		}

		static inline T&	ref()
		{
			validate_instance();
			return static_cast<T&>(*sm_instance);
		}

		static inline T&	safe_ref()
		{
			std::lock_guard glock(sm_mutex);
			validate_instance();
			return static_cast<T&>(*sm_instance);
		}

		template<typename U>
		static inline U*	get_as()
		{
			return static_cast<U*>(sm_instance);
		}

	protected: // lifecycle
		CLASS_CTOR			Singleton()
		{
			std::lock_guard glock(sm_mutex);

			if (sm_instance) 
				throw GeneralException(this, __LINE__, std::string("Only one instance of ") + typeid(T).name() + " can be created at a time.");

			sm_instance = this;
		}

		CLASS_DTOR			~Singleton()
		{
			try
			{
				std::lock_guard glock(sm_mutex);
				sm_instance = nullptr;
			}
			catch(...)
			{
				sm_instance = nullptr;
			}
		}

	private: // lifecycle
		CLASS_CTOR			Singleton(		const Singleton&	other) = delete; 

		CLASS_CTOR			Singleton(		Singleton&&			other) = delete; 

		Singleton&			operator=(		const Singleton&	other) = delete;

		Singleton&			operator=(		Singleton&&			other) = delete;

	private: // data
		static std::mutex sm_mutex;
		static Singleton* sm_instance;

	private: // functions
		static inline void	validate_instance()
		{
#ifdef _DEBUG
			if(!sm_instance)
				throw dpl::GeneralException("Fail to access singleton instance: " + std::string(typeid(T).name()));
#endif // _DEBUG
		}
	};

	template<class T>
	std::mutex		Singleton<T>::sm_mutex		= std::mutex();

	template<class T>
	Singleton<T>*	Singleton<T>::sm_instance	= nullptr;
}