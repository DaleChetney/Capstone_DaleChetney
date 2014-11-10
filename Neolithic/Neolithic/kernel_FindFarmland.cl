bool goodForFarming(__read_only image2d_t height, __read_only image2d_t inRoads, const int2 pos, const float levelNess, const int range)
{
	float level = read_imagef(height,sampler,(int2)(pos.x,pos.y)).x;
	bool result=true;
	for (int i = -range; i < range+1; i++)
	{
		for (int j = -range; j < range+1; j++)
		{
			float4 check = read_imagef(inRoads,sampler,(int2)(pos.x+i,pos.y+j));
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

bool goodForHunting(__read_only image2d_t inRoads, const int2 pos,  const int range)
{
	bool result=true;
	for (int i = -range; i < range+1; i++)
	{
		for (int j = -range; j < range+1; j++)
		{
			float4 check = read_imagef(inRoads,sampler,(int2)(pos.x+i,pos.y+j));
			if(check.x>0.5f||check.y>0.5f)
			{
				result = false;
				break;
			}
		}
		if(!result)break;
	}
	return result;
}

__kernel void FindGoodLand (__read_only image2d_t height, __read_only image2d_t fertility,  __read_only image2d_t density, __read_only image2d_t inRoads, __global int* pop, __global int* tribe, const int seed,const int id, const float soil,const int range)
{
	int2 home = {pop[id*10+6],pop[id*10+7]};
	const int2 pos = {get_global_id(0), get_global_id(1)};
	if(pos.x>0&&pos.y>0&&pos.x<1023&&pos.y<1023)
	{
		int random = randomGen2Di(randomGen1Di(seed,id,1000),pos,100);
		bool found = false;
		if(random == 50)
		{
			if(pos.x>home.x-range&&pos.x<home.x+range&&pos.y>home.y-range&&pos.y<home.y+range)	
			{
				if(read_imagef(fertility,sampler,pos).y*soil>0.7f&&read_imagef(density,sampler,pos).x<0.7f)
				{
					if(goodForFarming(height,inRoads,pos,0.01f,5))						
					{
						pop[id*10+2] = pos.x;
						pop[id*10+3] = pos.y;
						found = true;
					}
				}
			}
			//if(!found)
			//{
			//	range = 100;
			//	if(pos.x>home.x-range&&pos.x<home.x+range&&pos.y>home.y-range&&pos.y<home.y+range)	
			//	{
			//		if(read_imagef(fertility,sampler,pos).y>0.5f&&read_imagef(density,sampler,pos).x>0.5f)
			//		{
			//			if(goodForHunting(inRoads,pos,10))						
			//			{
			//				pop[id*10+2] = pos.x;
			//				pop[id*10+3] = pos.y;
			//			}
			//		}
			//	}
			//}
		}
	}
}