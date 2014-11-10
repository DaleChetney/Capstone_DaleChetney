bool goodForCamp(__read_only image2d_t height, __read_only image2d_t density, const int2 pos, const float levelNess, const int range)
{
	float level = read_imagef(height,sampler,(int2)(pos.x,pos.y)).x;
	bool result=true;
	for (int i = -range; i < range+1; i++)
	{
		for (int j = -range; j < range+1; j++)
		{
			float4 check = read_imagef(density,sampler,(int2)(pos.x+i,pos.y+j));
			if((fabs(read_imagef(height,sampler,(int2)(pos.x+i,pos.y+j)).x-level) > levelNess)||check.x>0.5f)
			{
				result = false;
				break;
			}
		}
		if(!result)break;
	}
	return result;
}

__kernel void Settle (__read_only image2d_t height, __read_only image2d_t fertility,  __read_only image2d_t density, __write_only image2d_t inRoads, __global int* tribe, const int seed,const int id)
{
	int2 home = {0,0};
	home.x = randomGen1Di(seed,id,1019)+5;
	home.y = randomGen1Di(seed*home.x,id,1019)+5;

	const int2 pos = {get_global_id(0), get_global_id(1)};
	int range =200;
	
	if(pos.x>home.x-range&&pos.x<home.x+range&&pos.y>home.y-range&&pos.y<home.y+range)
	{
		if(read_imagef(fertility,sampler, pos).y>0.7f&&read_imagef(density,sampler, pos).x<0.7f)
		{
			if(goodForCamp(height,density,pos,0.01f,5))
			{
				tribe[id*6+1] = pos.x;
				tribe[id*6+2] = pos.y;
				tribe[id*6+0] = 0;
				tribe[id*6+3] = 50+randomGen2Di(seed,pos,150);
				tribe[id*6+4] = 0;
				tribe[id*6+5] = 0;
			}
		}
	}
}