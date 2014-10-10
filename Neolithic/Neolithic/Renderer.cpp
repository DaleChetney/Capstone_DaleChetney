#include <GL\glew.h>
#include "Renderer.h"

Renderer* Renderer::instance;

Renderer* Renderer::getRenderer()
{
	if(instance == NULL)
	{
		instance = new Renderer;
	}
	return instance;
}

int Renderer::currentGeometry=0;
int Renderer::currentRenderable=0;
int Renderer::currentShader=0;
int Renderer::currentTexture=0;
int Renderer::currentFrameBuffer=0;
int Renderer::currentHudElement=0;

void Renderer::initializeGL()
{
	glewInit();
	glEnable(GL_DEPTH_TEST);
	
	setMouseTracking(true);
	clear();
}

Renderer::GeometryInfo* Renderer::addGeometry(Neumont::ShapeData shape)
{
	return addGeometry(shape.verts, shape.vertexBufferSize(),shape.indices,shape.numIndices,GL_TRIANGLES);
}

Renderer::GeometryInfo* Renderer::addGeometry(const char* fileName)
{
	ifstream file(fileName, std::ios::in|std::ios::binary);

	file.seekg(0, std::ios::end);
	unsigned int numBytes = file.tellg();
	char* buff = new char[numBytes];
	file.seekg(0, std::ios::beg);
	file.read(buff, numBytes);

	unsigned int numVerts = * reinterpret_cast<unsigned int*>(buff);
	unsigned int numIndices = * reinterpret_cast<unsigned int*>(buff + sizeof(unsigned int));
	Vertex* verts = reinterpret_cast<Vertex*>(buff + 2 * sizeof(unsigned int));

	unsigned short* indices = reinterpret_cast<unsigned short*>(buff + 2 * sizeof(unsigned int) + sizeof(Vertex) * numVerts);

	GeometryInfo& result = *addGeometry(verts, numVerts*sizeof(Vertex),indices,numIndices,GL_TRIANGLES);
	return &result;
}

Renderer::GeometryInfo* Renderer::addGeometry(void* verts,
	GLuint vertexDataSize, ushort* indices, uint numIndices, GLuint indexingMode)
{
	int currentBuffer;
	for(int i=0; i<NUM_BUFFERS;i++)
	{
		if (bufferInfos[i].remainingSize>vertexDataSize + sizeof(ushort)*numIndices) 
		{
			currentBuffer=i;
			break;
		}
	}
	BufferInfo& buffer = bufferInfos[currentBuffer];

	if(buffer.remainingSize==BUFFER_SIZE)
	{
		buffer.offset=0;
		glGenBuffers(1,&buffer.glBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, buffer.glBufferID);
		glBufferData(GL_ARRAY_BUFFER,buffer.remainingSize,0,GL_STATIC_DRAW);
	}

	buffer.remainingSize -= vertexDataSize + sizeof(ushort)*numIndices;

	glBufferSubData(GL_ARRAY_BUFFER, buffer.offset, vertexDataSize,verts);
	glBufferSubData(GL_ARRAY_BUFFER, buffer.offset+vertexDataSize, sizeof(ushort)*numIndices,indices);

	GeometryInfo& result = geometryInfos[currentGeometry++];
	glGenVertexArrays(1, &result.vertexArrayID);
	result.vertData = reinterpret_cast<Vertex*>(verts);
	result.indexData = indices;
	result.numVerts = vertexDataSize/sizeof(Vertex);
	result.numIndices = numIndices;
	result.indexingMode=indexingMode;
	result.buffer = &buffer;
	result.arrayOffset = buffer.offset;
	result.indexOffset = buffer.offset + vertexDataSize;

	buffer.offset += sizeof(ushort)*numIndices + vertexDataSize;

	return &result;
}

void Renderer::addShaderStreamedParameter(GeometryInfo* geometry, uint layoutLocation,
	ParameterType parameterType, uint bufferOffset, uint bufferStride)
{
	glBindVertexArray(geometry->vertexArrayID);
	glEnableVertexAttribArray(layoutLocation);
	glBindBuffer(GL_ARRAY_BUFFER, geometry->buffer->glBufferID);

	glVertexAttribPointer(layoutLocation, parameterType/sizeof(float),
		GL_FLOAT, GL_FALSE, bufferStride,(void*)(geometry->arrayOffset+bufferOffset));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->buffer->glBufferID);
}

Renderer::ShaderInfo* Renderer::createShaderInfo(const char* vertexShaderFileName,const char* fragmentShaderFileName)
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const char* fixGL[1];
	string temp = readShaderCode(vertexShaderFileName);
	fixGL[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, fixGL, 0);
	temp = readShaderCode(fragmentShaderFileName);
	fixGL[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, fixGL, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	GLint compileStatus;
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &compileStatus);
	if(compileStatus != GL_TRUE)
	{
		GLint logLength;
		glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &logLength);
		char* buffer = new char[logLength];
		GLsizei bits;
		glGetShaderInfoLog(vertexShaderID, logLength, &bits, buffer);
		qDebug() << buffer;
		delete [] buffer;
	}
	ShaderInfo& result = shaderInfos[currentShader++];
	result.programID = glCreateProgram();
	glAttachShader(result.programID, vertexShaderID);
	glAttachShader(result.programID, fragmentShaderID);
	glLinkProgram(result.programID);
	return &result;
}

void Renderer::addRenderableUniformParameter(RenderableInfo* renderable, const char* name,
	ParameterType parameterType, const float* value)
{
	ShaderUniformParameter& uniform = renderable->uniformParameters[renderable->numUniformParameters++];
	uniform.location = glGetUniformLocation(renderable->howShader->programID,name);
	uniform.parameterType=parameterType;
	uniform.value=value;
}

void Renderer::addHUDUniformParameter(HUDElement* element, const char* name,
	ParameterType parameterType, const float* value)
{
	ShaderUniformParameter& uniform = element->uniformParameters[element->numUniformParameters++];
	uniform.location = glGetUniformLocation(element->howShader->programID,name);
	uniform.parameterType=parameterType;
	uniform.value=value;
}

void Renderer::addShaderUniformParameter(ShaderInfo* shader, const char* name,
	ParameterType parameterType, const float* value)
{
	ShaderUniformParameter& uniform = shader->uniformParameters[shader->numUniformParameters++];
	uniform.location = glGetUniformLocation(shader->programID,name);
	uniform.parameterType=parameterType;
	uniform.value=value;
}

void Renderer::addShaderMap(ShaderInfo* shader,const char* name,GLint id)
{
	ShaderMap& map = shader->maps[shader->numMaps];
	map.location = glGetUniformLocation(shader->programID,name);
	map.mapID = id;
	map.position = shader->numMaps++;
}

void Renderer::addRenderableMap(RenderableInfo* renderable,const char* name,GLint id)
{
	ShaderMap& map = renderable->maps[renderable->numMaps];
	map.location = glGetUniformLocation(renderable->howShader->programID,name);
	map.mapID=id;
	map.position=renderable->numMaps++;
}

void Renderer::addHUDMap(HUDElement* element,const char* name,GLint id)
{
	ShaderMap& map = element->maps[element->numMaps];
	map.location = glGetUniformLocation(element->howShader->programID,name);
	map.mapID=id;
	map.position=element->numMaps++;
}

void Renderer::setRenderableUniformParameter(ShaderUniformParameter uniform)
{
	if (uniform.parameterType==ParameterType::PT_FLOAT) 
		glUniform1f(uniform.location,*uniform.value);
	if (uniform.parameterType==ParameterType::PT_VEC2) 
		glUniform2fv(uniform.location,1,uniform.value);
	if (uniform.parameterType==ParameterType::PT_VEC3) 
		glUniform3fv(uniform.location,1,uniform.value);
	if (uniform.parameterType==ParameterType::PT_VEC4) 
		glUniform4fv(uniform.location,1,uniform.value);
	if (uniform.parameterType==ParameterType::PT_MAT3) 
		glUniformMatrix3fv(uniform.location,1,GL_FALSE,uniform.value);
	if (uniform.parameterType==ParameterType::PT_MAT4) 
		glUniformMatrix4fv(uniform.location,1,GL_FALSE,uniform.value);
}

void Renderer::setMap(ShaderMap map)
{
	glActiveTexture(GL_TEXTURE0+map.position);
	glBindTexture(GL_TEXTURE_2D,map.mapID);
	glUniform1i(map.location,map.position);
}

Renderer::TextureInfo* Renderer::addTexture(const char* filename)
{
	char header[54];
	int  width, height, size;
	char* data;

#pragma warning( disable : 4996)
	FILE* file = fopen(filename,"rb");
#pragma warning( default : 4996)

	if(!file)
	{
		printf("Image could not be opened");
		return 0;
	}
	if((fread(header, 1, 54, file)!=54)||(header[0]!='B'||header[1]!='M'))
	{
		printf("Incorrect file formating.");
		return 0;
	}

	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	size=width*height*3;

	data = new char[size];
	fread(data,1,size,file);
	fclose(file);

	TextureInfo& result = textureInfos[currentTexture++];

	glGenTextures(1, &result.textureID);
	glBindTexture(GL_TEXTURE_2D,result.textureID);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width,height,0,GL_BGR,GL_UNSIGNED_BYTE,data);

	return &result;
}

Renderer::TextureInfo* Renderer::addNoiseMap(float lowerX, float upperX, float lowerY, float upperY, float lowerZ, float upperZ, int width, int height, int octaves)
{
	noise::module::Perlin myModule;

	byte* data = new byte[width*height*4];

	float xRange = upperX-lowerX;
	float yRange = upperY-lowerY;
	float zRange = upperZ-lowerZ;

	float xFactor = xRange/width;
	float yFactor = yRange/height;
	float zFactor = zRange/height;

	for (int oct = 0; oct<4; oct++)
	{
		myModule.SetOctaveCount(octaves);
		for (int i = 0; i<width; i++)
		{
			for (int j = 0; j<height; j++)
			{
				float x = lowerX + xFactor * i;
				float y = lowerY + yFactor * j;
				float z = lowerZ + zFactor * j;

				float val = (float)myModule.GetValue((double)x,(double)y,(double)z);
				val = (val + 1.0f) * 0.5f;
				val = (val>1.0f)? 1.0f : val;
				val = (val<0.0f)? 0.0f : val;
				data[((j*width + i) * 4)+oct] = (byte)(val*255.0f);
			}
		}
	}

	TextureInfo& result = textureInfos[currentTexture++];

	glGenTextures(1, &result.textureID);
	glBindTexture(GL_TEXTURE_2D,result.textureID);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	return &result;
}

void Renderer::updateNoiseMap(int mapId, float lowerX, float upperX, float lowerY, float upperY, float lowerZ, float upperZ, int width, int height, int octaves)
{
	noise::module::Perlin myModule;

	byte* data = new byte[width*height*4];

	float xRange = upperX-lowerX;
	float yRange = upperY-lowerY;
	float zRange = upperZ-lowerZ;

	float xFactor = xRange/width;
	float yFactor = yRange/height;
	float zFactor = zRange/height;

	for (int oct = 0; oct<4; oct++)
	{
		myModule.SetOctaveCount(octaves);
		for (int i = 0; i<width; i++)
		{
			for (int j = 0; j<height; j++)
			{
				float x = lowerX + xFactor * i;
				float y = lowerY + yFactor * j;
				float z = lowerZ + zFactor * j;

				float val = (float)myModule.GetValue((double)x,(double)y,(double)z);
				val = (val + 1.0f) * 0.5f;
				val = (val>1.0f)? 1.0f : val;
				val = (val<0.0f)? 0.0f : val;
				data[((j*width + i) * 4)+oct] = (byte)(val*255.0f);
			}
		}
	}

	glBindTexture(GL_TEXTURE_2D,mapId);

	glTexSubImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);

}

Renderer::RenderableInfo* Renderer::addRenderable(GeometryInfo* whatGeometry,
	const mat4& whereMatrix, const mat4& rotation, ShaderInfo* howShaders,
	TextureInfo* texture, bool transparent, GLuint transparencyConstant,
	const vec3& debugColor, bool depthEnabled, int lifespan, float debugType)
{
	RenderableInfo& shape = renderableInfos[currentRenderable++];
	shape.whatGeometry=whatGeometry;
	shape.whereMatrix=whereMatrix;
	shape.rotation = rotation;
	shape.howShader=howShaders;
	shape.depthEnabled = depthEnabled;
	shape.debugColor = debugColor;
	shape.lifeTime = lifespan;
	shape.debugType = debugType;
	shape.transparent = transparent;
	shape.transparencyConstant = transparencyConstant;
	addRenderableUniformParameter(&shape,"mtwTransform",ParameterType::PT_MAT4,&shape.whereMatrix[0][0]);
	addRenderableUniformParameter(&shape,"rotation",ParameterType::PT_MAT4,&shape.rotation[0][0]);
	addRenderableUniformParameter(&shape, "color", ParameterType::PT_VEC3, &shape.debugColor[0]);
	addRenderableUniformParameter(&shape, "specialCase", ParameterType::PT_FLOAT, &shape.debugType);
	if (texture!=NULL)addRenderableMap(&shape, "textureSample",texture->textureID);
	return &shape;
}

Renderer::HUDElement* Renderer::addHUDElement(GeometryInfo* whatGeometry,
	const mat4& whereMatrix, const mat4& rotation, ShaderInfo* howShaders,
	TextureInfo* texture, bool transparent, GLuint transparencyConstant,
	const vec3& debugColor, bool depthEnabled, int lifespan, float debugType)
{
	HUDElement& element = HUDElements[currentHudElement++];
	element.whatGeometry=whatGeometry;
	element.whereMatrix=whereMatrix;
	element.rotation = rotation;
	element.howShader=howShaders;
	element.depthEnabled = depthEnabled;
	element.debugColor = debugColor;
	element.lifeTime = lifespan;
	element.debugType = debugType;
	element.transparent = transparent;
	element.transparencyConstant = transparencyConstant;
	addHUDUniformParameter(&element,"mtwTransform",ParameterType::PT_MAT4,&element.whereMatrix[0][0]);
	addHUDUniformParameter(&element,"rotation",ParameterType::PT_MAT4,&element.rotation[0][0]);
	addHUDUniformParameter(&element, "color", ParameterType::PT_VEC3, &element.debugColor[0]);
	addHUDUniformParameter(&element, "specialCase", ParameterType::PT_FLOAT, &element.debugType);
	if (texture!=NULL)addHUDMap(&element, "textureSample",texture->textureID);
	return &element;
}

void Renderer::addFrameBuffer()
{
	glGenFramebuffers(1, &FrameBufferInfos[currentFrameBuffer].FrameBufferID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FrameBufferInfos[currentFrameBuffer].FrameBufferID);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &textureInfos[currentTexture].textureID);
	glBindTexture(GL_TEXTURE_2D, textureInfos[currentTexture].textureID);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureInfos[currentTexture++].textureID, 0);
	glGenTextures(1, &textureInfos[currentTexture].textureID);
	glBindTexture(GL_TEXTURE_2D, textureInfos[currentTexture].textureID);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width(), height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureInfos[currentTexture++].textureID, 0);

	GLuint status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE) qDebug() << "Frame buffer incomplete.";
}

string Renderer::readShaderCode(const char* file)
{
	ifstream input(file);
	if(! input.good())
	{
		std::cout << "I can't load " << file;
		exit(1);
	}
	return string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

void Renderer::paintGL()
{
	for(int buf=2; buf>=0; buf--)
	{
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER,buf);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0,0, width(), height());
		
		for (int i = 0; i < currentRenderable; i++)
		{
			RenderableInfo* shape = &renderableInfos[i];
			if (shape->visible&&shape->depthEnabled&&!shape->transparent&&!(shape->maps[0].mapID>=24&&buf>0))
			{
				glUseProgram(shape->howShader->programID);
				glBindVertexArray(shape->whatGeometry->vertexArrayID);

				for (uint i = 0; i < shape->howShader->numMaps; i++)
				{
					setMap(shape->howShader->maps[i]);
				}
				for (uint i = 0; i < shape->numMaps; i++)
				{
					setMap(shape->maps[i]);
				}
				for (uint i = 0; i < shape->howShader->numUniformParameters; i++)
				{
					setRenderableUniformParameter(shape->howShader->uniformParameters[i]);
				}
				for (uint i = 0; i < shape->numUniformParameters; i++)
				{
					setRenderableUniformParameter(shape->uniformParameters[i]);
				}
				glDrawElements(
					shape->whatGeometry->indexingMode,shape->whatGeometry->numIndices,
					GL_UNSIGNED_SHORT,(void*)shape->whatGeometry->indexOffset);
				if(shape->lifeTime>0)shape->lifeTime--;
				shape->visible = (shape->lifeTime!=0);
			}
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		for (int i = 0; i < currentRenderable; i++)
		{
			RenderableInfo* shape = &renderableInfos[i];
			if (shape->visible&&shape->depthEnabled&&shape->transparent)
			{
				glUseProgram(shape->howShader->programID);
				glBindVertexArray(shape->whatGeometry->vertexArrayID);
				for (uint i = 0; i < shape->howShader->numMaps; i++)
				{
					setMap(shape->howShader->maps[i]);
				}
				for (uint i = 0; i < shape->numMaps; i++)
				{
					setMap(shape->maps[i]);
				}
				for (uint i = 0; i < shape->howShader->numUniformParameters; i++)
				{
					setRenderableUniformParameter(shape->howShader->uniformParameters[i]);
				}
				for (uint i = 0; i < shape->numUniformParameters; i++)
				{
					setRenderableUniformParameter(shape->uniformParameters[i]);
				}
				glDrawElements(
					shape->whatGeometry->indexingMode,shape->whatGeometry->numIndices,
					GL_UNSIGNED_SHORT,(void*)shape->whatGeometry->indexOffset);
				if(shape->lifeTime>0)shape->lifeTime--;
				shape->visible = (shape->lifeTime!=0);
			}
		}
		glDisable(GL_BLEND);

		glDisable(GL_DEPTH_TEST);
		for (int i = 0; i < currentRenderable; i++)
		{
			RenderableInfo* shape = &renderableInfos[i];
			if (shape->visible&&!shape->depthEnabled)
			{
				glUseProgram(shape->howShader->programID);
				glBindVertexArray(shape->whatGeometry->vertexArrayID);

				for (uint i = 0; i < shape->howShader->numUniformParameters; i++)
				{
					setRenderableUniformParameter(shape->howShader->uniformParameters[i]);
				}
				for (uint i = 0; i < shape->numUniformParameters; i++)
				{
					setRenderableUniformParameter(shape->uniformParameters[i]);
				}
				glDrawElements(
					shape->whatGeometry->indexingMode,shape->whatGeometry->numIndices,
					GL_UNSIGNED_SHORT,(void*)shape->whatGeometry->indexOffset);
				if(shape->lifeTime>0)shape->lifeTime--;
				shape->visible = (shape->lifeTime!=0);
			}
		}

		for (int i = 0; i < currentHudElement; i++)
		{
			HUDElement* element = &HUDElements[i];
			if (element->visible&&element->depthEnabled&&!element->transparent)
			{
				glUseProgram(element->howShader->programID);
				glBindVertexArray(element->whatGeometry->vertexArrayID);

				for (uint i = 0; i < element->howShader->numMaps; i++)
				{
					setMap(element->howShader->maps[i]);
				}
				for (uint i = 0; i < element->numMaps; i++)
				{
					setMap(element->maps[i]);
				}
				for (uint i = 0; i < element->howShader->numUniformParameters; i++)
				{
					setRenderableUniformParameter(element->howShader->uniformParameters[i]);
				}
				for (uint i = 0; i < element->numUniformParameters; i++)
				{
					setRenderableUniformParameter(element->uniformParameters[i]);
				}
				glDrawElements(
					element->whatGeometry->indexingMode,element->whatGeometry->numIndices,
					GL_UNSIGNED_SHORT,(void*)element->whatGeometry->indexOffset);
				if(element->lifeTime>0)element->lifeTime--;
				element->visible = (element->lifeTime!=0);
			}
		}
		glEnable(GL_DEPTH_TEST);
	}
}

void Renderer::clear()
{
	for(int i=0; i<NUM_BUFFERS;i++)
	{
		bufferInfos[i].remainingSize=BUFFER_SIZE;
		bufferInfos[i].glBufferID=0;
		bufferInfos[i].offset=0;
	}
	for(int i=0; i<NUM_SHADERS;i++)
	{
		shaderInfos[i].programID=0;
		shaderInfos[i].numUniformParameters=0;
		shaderInfos[i].numMaps=0;
	}
	for(int i=0; i<NUM_GEOMETRIES;i++)
	{
		geometryInfos[i].vertexArrayID=0;
		geometryInfos[i].arrayOffset=0;
		geometryInfos[i].buffer=0;
		geometryInfos[i].indexingMode=0;
		geometryInfos[i].numIndices=0;
	}
	for(int i=0; i<NUM_TEXTURES;i++)
	{
		textureInfos[i].textureID=0;
	}
	for(int i=0; i<NUM_RENDERABLES;i++)
	{
		renderableInfos[i].numUniformParameters=0;
		renderableInfos[i].numMaps=0;
	}
	currentGeometry = 0; 
	currentShader = 0;
	currentTexture = 0;
	currentRenderable = 0;
}

Renderer::GeometryInfo* Renderer::makeCube()
{
VertexTangent verts[] = 
{
// Top
VertexTangent(
vec3(-1.0f, +1.0f, +1.0f), // 0
vec2(+0.0f, +1.0f), // UV
vec3(+0.0f, +1.0f, +0.0f), // Normal
vec3(+1.0f, +0.0f, +0.0f) // Tangent
),
VertexTangent(
vec3(+1.0f, +1.0f, +1.0f), // 1
vec2(+1.0f, +1.0f), // UV
vec3(+0.0f, +1.0f, +0.0f), // Normal
vec3(+1.0f, +0.0f, +0.0f) // Tangent
),
VertexTangent(
vec3(+1.0f, +1.0f, -1.0f), // 2
vec2(+1.0f, +0.0f), // UV
vec3(+0.0f, +1.0f, +0.0f), // Normal
vec3(+1.0f, +0.0f, +0.0f) // Tangent
),
VertexTangent(
vec3(-1.0f, +1.0f, -1.0f), // 3
vec2(+0.0f, +0.0f), // UV
vec3(+0.0f, +1.0f, +0.0f), // Normal
vec3(+1.0f, +0.0f, +0.0f) // Tangent
),


// Front
VertexTangent(
vec3(-1.0f, +1.0f, -1.0f), // 4
vec2(+0.0f, +1.0f), // UV
vec3(+0.0f, +0.0f, -1.0f), // Normal
vec3(+1.0f, +0.0f, -0.0f) // Tangent
),
VertexTangent(
vec3(+1.0f, +1.0f, -1.0f), // 5
vec2(+1.0f, +1.0f), // UV
vec3(+0.0f, +0.0f, -1.0f), // Normal
vec3(+1.0f, +0.0f, -0.0f) // Tangent
),
VertexTangent(
vec3(+1.0f, -1.0f, -1.0f), // 6
vec2(+1.0f, +0.0f), // UV
vec3(+0.0f, +0.0f, -1.0f), // Normal
vec3(+1.0f, +0.0f, -0.0f) // Tangent
),
VertexTangent(
vec3(-1.0f, -1.0f, -1.0f), // 7
vec2(+0.0f, +0.0f), // UV
vec3(+0.0f, +0.0f, -1.0f), // Normal
vec3(+1.0f, +0.0f, -0.0f) // Tangent
),


// Right
VertexTangent(
vec3(+1.0f, +1.0f, -1.0f), // 8
vec2(+1.0f, +0.0f), // UV
vec3(+1.0f, +0.0f, +0.0f), // Normal
vec3(+0.0f, +0.0f, -1.0f) // Tangent
),
VertexTangent(
vec3(+1.0f, +1.0f, +1.0f), // 9
vec2(+0.0f, +0.0f), // UV
vec3(+1.0f, +0.0f, +0.0f), // Normal
vec3(+0.0f, +0.0f, -1.0f) // Tangent
),
VertexTangent(
vec3(+1.0f, -1.0f, +1.0f), // 10
vec2(+0.0f, +1.0f), // UV
vec3(+1.0f, +0.0f, +0.0f), // Normal
vec3(+0.0f, +0.0f, -1.0f) // Tangent
),
VertexTangent(
vec3(+1.0f, -1.0f, -1.0f), // 11
vec2(+1.0f, +1.0f), // UV
vec3(+1.0f, +0.0f, +0.0f), // Normal
vec3(+0.0f, +0.0f, -1.0f) // Tangent
),


// Left
VertexTangent(
vec3(-1.0f, +1.0f, +1.0f), // 12
vec2(+1.0f, +0.0f), // UV
vec3(-1.0f, +0.0f, +0.0f), // Normal
vec3(-0.0f, +0.0f, +1.0f) // Tangent
),
VertexTangent(
vec3(-1.0f, +1.0f, -1.0f), // 13
vec2(+0.0f, +0.0f), // UV
vec3(-1.0f, +0.0f, +0.0f), // Normal
vec3(-0.0f, +0.0f, +1.0f) // Tangent
),
VertexTangent(
vec3(-1.0f, -1.0f, -1.0f), // 14
vec2(+0.0f, +1.0f), // UV
vec3(-1.0f, +0.0f, +0.0f), // Normal
vec3(-0.0f, +0.0f, +1.0f) // Tangent
),
VertexTangent(
vec3(-1.0f, -1.0f, +1.0f), // 15
vec2(+1.0f, +1.0f), // UV
vec3(-1.0f, +0.0f, +0.0f), // Normal
vec3(-0.0f, +0.0f, +1.0f) // Tangent
),


// Back
VertexTangent(
vec3(+1.0f, +1.0f, +1.0f), // 16
vec2(+1.0f, +0.0f), // UV
vec3(+0.0f, +0.0f, +1.0f), // Normal
vec3(+1.0f, +0.0f, +0.0f) // Tangent
),
VertexTangent(
vec3(-1.0f, +1.0f, +1.0f), // 17
vec2(+0.0f, +0.0f), // UV
vec3(+0.0f, +0.0f, +1.0f), // Normal
vec3(+1.0f, +0.0f, +0.0f) // Tangent
),
VertexTangent(
vec3(-1.0f, -1.0f, +1.0f), // 18
vec2(+0.0f, +1.0f), // UV
vec3(+0.0f, +0.0f, +1.0f), // Normal
vec3(+1.0f, +0.0f, +0.0f) // Tangent
),
VertexTangent(
vec3(+1.0f, -1.0f, +1.0f), // 19
vec2(+1.0f, +1.0f), // UV
vec3(+0.0f, +0.0f, +1.0f), // Normal
vec3(+1.0f, +0.0f, +0.0f) // Tangent
),

// Bottom
VertexTangent(
vec3(+1.0f, -1.0f, -1.0f), // 20
vec2(+1.0f, +1.0f), // UV
vec3(+0.0f, -1.0f, +0.0f), // Normal
vec3(+1.0f, -0.0f, +0.0f) // Tangent
),
VertexTangent(
vec3(-1.0f, -1.0f, -1.0f), // 21
vec2(+0.0f, +1.0f), // UV
vec3(+0.0f, -1.0f, +0.0f), // Normal
vec3(+1.0f, -0.0f, +0.0f) // Tangent
),
VertexTangent(
vec3(-1.0f, -1.0f, +1.0f), // 22
vec2(+0.0f, +0.0f), // UV
vec3(+0.0f, -1.0f, +0.0f), // Normal
vec3(+1.0f, -0.0f, +0.0f) // Tangent
),
VertexTangent(
vec3(+1.0f, -1.0f, +1.0f), // 23
vec2(+1.0f, +0.0f), // UV
vec3(+0.0f, -1.0f, +0.0f), // Normal
vec3(+1.0f, -0.0f, +0.0f) // Tangent
)
};
unsigned short indices[] = {
0, 1, 2, 0, 2, 3, // Top
4, 5, 6, 4, 6, 7, // Front
8, 9, 10, 8, 10, 11, // Right 
12, 13, 14, 12, 14, 15, // Left
16, 17, 18, 16, 18, 19, // Back
20, 22, 21, 20, 23, 22, // Bottom
};

return addGeometry(verts, sizeof(VertexTangent)*24,indices,36,GL_TRIANGLES);
}