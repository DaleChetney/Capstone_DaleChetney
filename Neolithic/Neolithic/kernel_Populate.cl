__kernel void Populate (__global int* pop, __global int* tribe, const int seed)
{
    const int id = get_global_id(0);
	pop[id*10+9] = randomGen1Di(seed,id,5);
	pop[id*10+0] = 0;
	pop[id*10+1] = 0;
	pop[id*10+2] = 0;
	pop[id*10+3] = 0;
	pop[id*10+4] = 0;
	pop[id*10+5] = 0;
	pop[id*10+6] = tribe[pop[id*10+9]*6+1] + randomGen1Di(seed,id,20)-10;
	pop[id*10+7] = tribe[pop[id*10+9]*6+2] + randomGen1Di(seed*pop[id*8+0],id,20)-10;
	pop[id*10+8] = randomGen1Di(seed,id,50);
	tribe[pop[id*10+9]*6+0]++;
}