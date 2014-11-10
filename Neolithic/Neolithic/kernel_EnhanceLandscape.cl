__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;


float randomGen1Df(const int globalSeed, const int id , const int range)
{
	uint seed = id;
	uint t  = seed ^ (seed << 11);
	uint result = globalSeed ^ (globalSeed >> 19) ^ (t ^ (t >> 8));
	result = result%1000;
	return (result/1000.0f)*range;
}

int randomGen1Di(const int globalSeed, const int id , const int range)
{
	uint seed = id;
	uint t  = seed ^ (seed << 11);
	uint result = globalSeed ^ (globalSeed >> 19) ^ (t ^ (t >> 8));
	return result%range;
}

float randomGen2Df(const int globalSeed, const int2 pos, const int range)
{
	uint seed = pos.x+(pos.y*1024);
	uint t  = seed ^ (seed << 11);
	uint result = globalSeed ^ (globalSeed >> 19) ^ (t ^ (t >> 8));
	result = result%1000;
	return (result/1000.0f)*range;
}

int randomGen2Di(const int globalSeed, const int2 pos, const int range)
{
	uint seed = pos.x+(pos.y*1024);
	uint t  = seed ^ (seed << 11);
	uint result = globalSeed ^ (globalSeed >> 19) ^ (t ^ (t >> 8));
	return result%range;
}


__kernel void Enhance (__read_only image2d_t input, __read_only image2d_t height, __read_only image2d_t fertility,  __read_only image2d_t density, __write_only image2d_t output)
{
    const int2 pos = {get_global_id(0), get_global_id(1)};
    float4 current = read_imagef(input,sampler,pos);
	float4 heightColor = read_imagef(height,sampler,pos);
	float4 fertColor = read_imagef(fertility,sampler,pos);
	float4 denseColor = read_imagef(density,sampler,pos);
	float4 newColor = {current.x+(fertColor.y/5), current.y-(denseColor.y/10)+(fertColor.y/5), current.z-(denseColor.z/10), current.w};
	write_imagef(output, pos, newColor);
}