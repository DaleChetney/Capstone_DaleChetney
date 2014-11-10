__kernel void DrawRoads (__read_only image2d_t inRoads, __read_only image2d_t height, __read_only image2d_t density, __write_only image2d_t outRoads, __global int* pop, __global int* tribe, const int seed)
{
	const int id = get_global_id(0);
	if(pop[id*10+2]!=0)
	{
		float level = read_imagef(height,sampler,(int2)(pop[id*10+0],pop[id*10+1])).x;
		float best=-10000000.0f;
		
		int yDist=abs(pop[id*10+3]-pop[id*10+1]);
		int xDist=abs(pop[id*10+2]-pop[id*10+0]);

		float dist = sqrt((float)(yDist*yDist+xDist*xDist));
		
		int2 bestChoice = {pop[id*10+0],pop[id*10+1]};
		
		for(int i = -1; i < 2; i++)
		{
			for(int j=-1;j<2; j++)
			{
				if(i!=0||j!=0)
				{
					float score = 0.0f;
		
					int2 checkPos = {pop[id*10+0]+j,pop[id*10+1]+i};
		
					//float checkLevel = read_imagef(height,sampler,checkPos).x;
					//score -= checkLevel*checkLevel*1.0f;
					//if(checkLevel<=0.0f) score -= 50.0f;
					//float diff = fabs(checkLevel-level)*1.0f;
					//score -= diff*diff*diff;

					//float checkForest = read_imagef(density,sampler,checkPos).x;
					//score -= checkForest*20.0f;
		
					float pathWear = read_imagef(inRoads,sampler,checkPos).x;
					if(pathWear>0.0f)score+=1.0f;
					pathWear = pathWear*1.0f;
					score += pathWear;
		
					if(i==0||j==0)score += 0.4f;

					if(read_imagef(inRoads,sampler,checkPos).z>0.0f)score-=10.0f;
					if(read_imagef(inRoads,sampler,checkPos).y>0.0f)score-=4.0f;
					
					int checkY = abs(pop[id*10+3] - checkPos.y);
					int checkX = abs(pop[id*10+2] - checkPos.x);
					float check = sqrt((float)(checkX*checkX+checkY*checkY));
					float progress = dist-check;
					score += progress;
		
					if(score > best&&!(pop[id*10+4]==checkPos.x&&pop[id*10+5]==checkPos.y))
					{
						bestChoice = checkPos;
						best = score;
					}
				}
			}
		}
		pop[id*10+4] = pop[id*10+0];
		pop[id*10+5] = pop[id*10+1];
		pop[id*10+0] = bestChoice.x;
		pop[id*10+1] = bestChoice.y;
		if(pop[id*10+0] == pop[id*10+2]&&pop[id*10+1] == pop[id*10+3])
		{
			for (int i = -2; i < 2+1; i++)
			{
				for (int j = -2; j < 2+1; j++)
				{
					write_imagef(outRoads,(int2)(pop[id*10+2]+i,pop[id*10+3]+j),(float4)(0.0f,1.0f,0.0f,1.0f));
				}
			}
			pop[id*10+2] = 0;
			pop[id*10+3] = 0;
			pop[id*10+0] = pop[id*10+6];
			pop[id*10+1] = pop[id*10+7];
			pop[id*10+4] = pop[id*10+6];
			pop[id*10+5] = pop[id*10+7];
			tribe[pop[id*10+9]*6+5]+=2;
		}
		float4 current = read_imagef(inRoads,sampler,bestChoice);
		write_imagef(outRoads,bestChoice,current + (float4)(0.072f,0.0f,0.0f,0.0f));
	}
}