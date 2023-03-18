#pragma once


#include "complex_Objects.h"

#pragma pack(push, 4)

// test objects
namespace complex_demo // declaration
{
	class ObjectA;
	class ObjectB;
	class ObjectC;
	class ObjectD;
}
namespace complex // specialization
{
	template<>
	struct Traits<complex_demo::ObjectA>
	{
		using ParentB			= complex::Bond<ONE_TO_ONE, complex_demo::ObjectB>;
		using ParentCD			= complex::Bond<ONE_TO_ONE, complex_demo::ObjectC, complex_demo::ObjectD>;

	public: // specialization
		using ParentTypes		= dpl::TypeList<ParentB, ParentCD>;
		using ChildTypes		= dpl::TypeList<>;
		using ComponentTypes	= dpl::TypeList<double, int>;
	};

	template<>
	struct Traits<complex_demo::ObjectB>
	{
		using ParentC			= complex::Bond<MANY_TO_ONE, complex_demo::ObjectC>;

	public: // specialization
		using ParentTypes		= dpl::TypeList<ParentC>;
		using ChildTypes		= dpl::TypeList<complex_demo::ObjectA>;
		using ComponentTypes	= dpl::TypeList<float>;
	};

	template<>
	struct Traits<complex_demo::ObjectC>
	{
		using ParentD			= complex::Bond<MANY_TO_ONE, complex_demo::ObjectD>;

	public: // specialization
		using ParentTypes		= dpl::TypeList<ParentD>;
		using ChildTypes		= dpl::TypeList<complex_demo::ObjectA, complex_demo::ObjectB>;
		using ComponentTypes	= dpl::TypeList<int32_t>;
	};

	template<>
	struct Traits<complex_demo::ObjectD>
	{
	public: // specialization
		using ParentTypes		= dpl::TypeList<>;
		using ChildTypes		= dpl::TypeList<complex_demo::ObjectA, complex_demo::ObjectC>;
		using ComponentTypes	= dpl::TypeList<int64_t, double>;
	};
}
namespace complex_demo // definition
{
	class ObjectA : public complex::Object<ObjectA>{public: using Object::Object;};
	class ObjectB : public complex::Object<ObjectB>{public: using Object::Object;};
	class ObjectC : public complex::Object<ObjectC>{public: using Object::Object;};
	class ObjectD : public complex::Object<ObjectD>{public: using Object::Object;};


	void perform()
	{
		dpl::CommandInvoker invoker;

		// Root objects may be generated through command:
		//auto* command = invoker.invoke<ObjectD::CreateCommand>(complex::GENERIC_NAME, "Object_");

		// New object can be accessed through command:
		//ObjectD& obj = *command->obj();

		//std::cout << "Created object name: " << obj.name() << std::endl;

		{ // component access

			// Components can be accessed through objects that "own" them (inefficient, but usefull for GUI).
			//obj.get<int64_t>()	= 1;
			//obj.get<double>()	= 1.0;
			// obj.get<uint32_t>(); <-- Fails due to 'uint32_t' type not being in the list of component types.

			complex::ObjectPack_of<ObjectD>::ref();
		}
	}
}

#pragma pack(pop)