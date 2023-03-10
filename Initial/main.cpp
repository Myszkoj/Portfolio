#include <iostream>
#include <dpl_ReadOnly.h>
#include <dpl_ClassInfo.h>
#include <dpl_Values.h>


class BankAccount
{
public: // subtypes
	using Value_t = double;
	using Dollars = dpl::RangedValue<Value_t, (Value_t)0, (Value_t)std::numeric_limits<float>::max()>;

public: // data
	dpl::ReadOnly<double, BankAccount> balance;

public: // lifecycle
	CLASS_CTOR	BankAccount()
		// intended error
	{

	}

	CLASS_CTOR	BankAccount(const double INITIAL_BALANCE)
		: balance(INITIAL_BALANCE)
	{

	}
};


int main(int argc, char** argv)
{
	BankAccount ba;

	return 0;
}