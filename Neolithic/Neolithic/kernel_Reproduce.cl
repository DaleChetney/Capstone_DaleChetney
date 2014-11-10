__kernel void Reproduce (__global int* pop, __global int* totalPeople, const int seed)
{
    const int id = get_global_id(0);
	if(pop[id*9+8]==20&&totalPeople[0]<1024)
	{
		totalPeople[0]++;
		pop[totalPeople[0]*9+0] = 0;
		pop[totalPeople[0]*9+1] = 0;
		pop[totalPeople[0]*9+2] = 0;
		pop[totalPeople[0]*9+3] = 0;
		pop[totalPeople[0]*9+4] = 0;
		pop[totalPeople[0]*9+5] = 0;
		pop[totalPeople[0]*9+6] = pop[id*9+6]+randomGen1Di(seed,id,3);
		pop[totalPeople[0]*9+7] = pop[id*9+7]+randomGen1Di(seed*pop[id*8+0],id,3);
		pop[totalPeople[0]*9+8] = 0;
	}
	if(pop[id*9+8]>=50)
	{
		pop[id*9+0] = 0;
		pop[id*9+1] = 0;
		pop[id*9+2] = 0;
		pop[id*9+3] = 0;
		pop[id*9+4] = 0;
		pop[id*9+5] = 0;
		pop[id*9+6] = randomGen1Di(seed,id,1024);
		pop[id*9+7] = randomGen1Di(seed*pop[id*8+0],id,1024);
		pop[id*9+8] = 0;
	}
}