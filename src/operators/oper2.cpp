#include "oper2.h"

oper2::oper2(variable *a, variable *b)
{
	arg1 = a;
	arg2 = b;
}

oper2::~oper2()
{
	delete arg1;
	delete arg2;
}