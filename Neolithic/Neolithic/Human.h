#pragma once
#include "CL\cl.h"
struct Human
{
	cl_int2 position;
	cl_int2 destination;
	cl_int2 previousPosition;
	cl_int2 homePosition;
	cl_int age;
	cl_int tribe;
};