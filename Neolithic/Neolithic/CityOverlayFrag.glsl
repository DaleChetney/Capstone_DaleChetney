#version 400
	
in vec2 outUV;
out vec4 finalColor;

uniform sampler2D textureSample;

void main()
{
	finalColor =  texture(textureSample,outUV);
	if(finalColor.x==0.0f&&finalColor.y==0.0f&&finalColor.z==0.0f)
	{
		discard;
	}
}