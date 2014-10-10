__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void RedDots (__read_only image2d_t input, __write_only image2d_t output)
{
    const int2 pos = {get_global_id(0), get_global_id(1)};
	int2 rightcheck = {pos.x+1,pos.y};
	int speed = (read_imagef(input,sampler,rightcheck).z>0.65f)? 1:2;
    float4 current = read_imagef(input,sampler,pos);
	if(current.x==1.0f)
	{
		int2 rightpos = {pos.x+speed,pos.y};
		float4 right = read_imagef(input,sampler,rightpos);
		if(right.x<=0.1f)
		{
			write_imagef (output, rightpos, current);
			write_imagef (output, pos, right);
		}
		else
		{
			int2 downpos = {pos.x,pos.y-speed};
			float4 down = read_imagef(input,sampler,downpos);
			if(down.x<=0.1f)
			{
				write_imagef (output, downpos, current);
				write_imagef (output, pos, down);
			}
			else
			{
				int2 backpos[50];

				for(int i=0; i<50; i++)
				{
					backpos[i].x = pos.x-(i+1);
					backpos[i].y = pos.y-speed;
				}

				for(int j=0; j<50; j++)
				{
					if(read_imagef(input,sampler,backpos[j]).x<=0.1f)
					{
						write_imagef (output, backpos[j], current);
						write_imagef (output, pos, read_imagef(input,sampler,backpos[j]));
						break;
					}
				}
			}
		}
	}
}