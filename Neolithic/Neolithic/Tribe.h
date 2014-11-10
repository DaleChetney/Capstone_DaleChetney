#pragma once
#include "CL\cl.h"
struct Tribe
{
	cl_int population;
	cl_int2 center;
	cl_int range;
	cl_int storedFood;
	cl_int annualHarvest; 
};