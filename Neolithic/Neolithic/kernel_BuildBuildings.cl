void drawSquare(__write_only image2d_t out, const int2 pos, const int range)
{
	for (int i = -range; i < range+1; i++)
	{
		for (int j = -range; j < range+1; j++)
		{
			write_imagef(out,(int2)(pos.x+i,pos.y+j),(float4)(0.0f,0.0f,1.0f,1.0f));
		}
	}
}

bool goodForBuilding(__read_only image2d_t in, const int2 pos, const int range)
{
	bool result=true;
	for (int i = -range; i < range+1; i++)
	{
		for (int j = -range; j < range+1; j++)
		{
			float4 check = read_imagef(in,sampler,(int2)(pos.x+i,pos.y+j));
			if(check.x>0.0f||check.y>0.0f||check.z>0.0f)
			{
				result = false;
				break;
			}
		}
		if(!result)break;
	}
	return result;
}

__kernel void BuildBuildings (__read_only image2d_t inRoads, __write_only image2d_t outCity, const int seed)
{
	const int2 pos = {get_global_id(0),get_global_id(1)};
	float4 traffic = read_imagef(inRoads, sampler, pos);
	float random = randomGen2Df(seed,pos,1)/2.0f+0.5f;
	if(traffic.x>random)
	{
		int direction = randomGen2Di(seed,pos,8);
		int2 dir = {0,0};
		if(direction == 0) dir = (int2)(pos.x,  pos.y+3);
		if(direction == 1) dir = (int2)(pos.x+3,pos.y+3);
		if(direction == 2) dir = (int2)(pos.x+3,pos.y);
		if(direction == 3) dir = (int2)(pos.x+3,pos.y-3);
		if(direction == 4) dir = (int2)(pos.x,  pos.y-3);
		if(direction == 5) dir = (int2)(pos.x-3,pos.y-3);
		if(direction == 6) dir = (int2)(pos.x-3,pos.y);
		if(direction == 7) dir = (int2)(pos.x-3,pos.y+3);
		if(goodForBuilding(inRoads, dir, 2))
		{
			drawSquare(outCity,dir,2);
		}
	}
	if(traffic.x>0.0f)write_imagef(outCity,pos,traffic+(float4)(-0.002f,0.0f,0.0f,0.0f));
	if(traffic.y>0.0f)write_imagef(outCity,pos,traffic+(float4)(0.0f,-0.002f,0.0f,0.0f));
	if(traffic.z>0.0f)write_imagef(outCity,pos,traffic+(float4)(0.0f,0.0f,-0.002f,0.0f));
}