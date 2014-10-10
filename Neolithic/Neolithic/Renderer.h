#pragma once
#include <QtOpenGL\qglwidget>
#include <glm\glm.hpp>
#include <ShapeGenerator.h>
using glm::mat4;
using glm::vec3;
using glm::vec2;
#include <iostream>
using std::string;
#include <fstream>
using std::ifstream;
#include <Qt\qdebug.h>
#include "noise\noise.h"

#define RENDERER Renderer::getRenderer()
#define Shader Renderer::ShaderInfo
#define Texture Renderer::TextureInfo
#define Geometry Renderer::GeometryInfo
#define ParamType Renderer::ParameterType
#define RENDER Renderer::RenderableInfo

class Renderer : public QGLWidget
{
public:
	static Renderer* getRenderer();
private:
	Renderer(){}
	Renderer(const Renderer&);
	Renderer& operator = (const Renderer&);
	static Renderer* instance;

	static const int BUFFER_SIZE=2048576, NUM_BUFFERS=10, NUM_GEOMETRIES=50,
			  NUM_SHADERS=20, NUM_TEXTURES=30, MAX_UNIFORM_PARAMETERS = 30,
			  NUM_RENDERABLES=1000,MAX_MAPS = 9,NUM_FRAME_BUFFERS=2,NUM_HUD_ELEMENTS=2;
			  
public:
	static int currentGeometry, currentShader, 
		currentTexture, currentUniform, currentMap, 
		currentFrameBuffer, currentHudElement;

	struct BufferInfo
	{
		GLuint glBufferID;
		GLuint remainingSize;
		GLuint offset;
	}bufferInfos[NUM_BUFFERS];
	

	struct Vertex
	{
		vec3 position;
		vec2 uv;
		vec3 normal;
	};

	struct VertexTangent
	{
		vec3 position;
		vec2 uv;
		vec3 normal;
		vec3 tangent;
		VertexTangent(vec3 pos, vec2 UV, vec3 norm, vec3 tan)
		{
			position = pos;
			uv = UV;
			normal = norm;
			tangent = tan;
		}
	};

	static int currentRenderable;
	static const int POSITION_OFFSET = 0, UV_OFFSET = sizeof(float)*3, NORMAL_OFFSET = sizeof(float)*5, STRIDE = sizeof(float)*8, TANGENT_OFFSET = sizeof(float)*8, TANGENT_STRIDE = sizeof(float)*11;

	enum ParameterType
	{
		PT_TEXTURE = 1,
		PT_FLOAT = sizeof(float) * 1,
		PT_VEC2 = sizeof(float) * 2,
		PT_VEC3 = sizeof(float) * 3,
		PT_VEC4 = sizeof(float) * 4,
		PT_MAT3 = sizeof(float) * 9,
		PT_MAT4 = sizeof(float) * 16,
	};

	struct FrameBufferInfo
	{
		GLuint FrameBufferID;
	}FrameBufferInfos[NUM_FRAME_BUFFERS];

	struct GeometryInfo
	{
		Vertex* vertData;
		ushort* indexData;
		GLuint vertexArrayID;
		GLuint arrayOffset;
		GLuint indexOffset;
		GLuint indexingMode;
		GLuint numVerts;
		GLuint numIndices;
		BufferInfo* buffer;
	}geometryInfos[NUM_GEOMETRIES];
	
	struct ShaderUniformParameter
	{
		GLint location;
		ParameterType parameterType;
		const float* value;
	};

	struct ShaderMap
	{
		GLint location;
		GLuint position;
		GLint mapID;
	};

	struct ShaderInfo
	{
		GLuint programID;
		GLuint numUniformParameters;
		GLuint numMaps;
		ShaderMap maps[MAX_MAPS];
		ShaderUniformParameter uniformParameters[MAX_UNIFORM_PARAMETERS];
	}shaderInfos[NUM_SHADERS];

	struct TextureInfo
	{
		GLuint textureID;
	}textureInfos[NUM_TEXTURES];

	struct HUDElement
	{
		GeometryInfo* whatGeometry;
		ShaderInfo* howShader;
		mat4 whereMatrix;
		mat4 rotation;
		bool visible;
		int lifeTime;
		bool depthEnabled;
		vec3 debugColor;
		GLuint numUniformParameters;
		GLuint numMaps;
		bool transparent;
		GLuint transparencyConstant;
		float debugType;
		ShaderMap maps[MAX_MAPS];
		ShaderUniformParameter uniformParameters[MAX_UNIFORM_PARAMETERS];
	}HUDElements[NUM_HUD_ELEMENTS];
	
	struct RenderableInfo
	{
		GeometryInfo* whatGeometry;
		ShaderInfo* howShader;
		mat4 whereMatrix;
		mat4 rotation;
		bool visible;
		int lifeTime;
		bool depthEnabled;
		vec3 debugColor;
		GLuint numUniformParameters;
		GLuint numMaps;
		bool transparent;
		GLuint transparencyConstant;
		float debugType;
		ShaderMap maps[MAX_MAPS];
		ShaderUniformParameter uniformParameters[MAX_UNIFORM_PARAMETERS];
	}renderableInfos[NUM_RENDERABLES];

	GeometryInfo* addGeometry(
		void* verts,
		GLuint vertexDataSize,
		ushort* indices, 
		uint numIndices, 
		GLuint indexingMode);
	GeometryInfo* addGeometry(Neumont::ShapeData shape);
	GeometryInfo* addGeometry(const char* fileName);

	ShaderInfo* createShaderInfo(
		const char* vertexShaderFileName,
		const char* fragmentShaderFileName);

	TextureInfo* addTexture(const char* filename);

	TextureInfo* addNoiseMap(float lowerX=0.0f, float upperX=1.0f, float lowerY=0.0f, float upperY=1.0f, float lowerZ=0.0f, float upperZ=0.0f, int width=512, int hieght=512, int octaves=6);

	void updateNoiseMap(int mapId, float lowerX=0.0f, float upperX=1.0f, float lowerY=0.0f, float upperY=1.0f, float lowerZ=0.0f, float upperZ=0.0f, int width=512, int hieght=512, int octaves=6);

	RenderableInfo* addRenderable(
		GeometryInfo* whatGeometry,
		const mat4& whereMatrix,
		const mat4& rotation,
		ShaderInfo* howShaders,
		TextureInfo* texture = NULL,
		bool transparent = false,
		GLuint transparencyConstant = 0,
		const vec3& debugColor = vec3(),
		bool depthEnabled = true,
		int lifetime = -1,
		float debugType = 0.0f);

	HUDElement* addHUDElement(
		GeometryInfo* whatGeometry,
		const mat4& whereMatrix,
		const mat4& rotation,
		ShaderInfo* howShaders,
		TextureInfo* texture = NULL,
		bool transparent = false,
		GLuint transparencyConstant = 0,
		const vec3& debugColor = vec3(),
		bool depthEnabled = true,
		int lifetime = -1,
		float debugType = 0.0f);

	void addShaderStreamedParameter(
		GeometryInfo* geometry, 
		uint layoutLocation,
		ParameterType parameterType,
		uint bufferOffset,
		uint bufferStride);

	void addShaderUniformParameter(
		ShaderInfo* shader,
		const char* name,
		ParameterType parameterType,
		const float* value);

	void addRenderableUniformParameter(
		RenderableInfo* renderable, 
		const char* name,
		ParameterType parameterType, 
		const float* value);

	void addHUDUniformParameter(
		HUDElement* element, 
		const char* name,
		ParameterType parameterType, 
		const float* value);

	void addShaderMap(
		ShaderInfo* shader,
		const char* name,
		GLint position);

	void addRenderableMap(
		RenderableInfo* renderable, 
		const char* name,
		GLint position);

	void addHUDMap(
		HUDElement* element, 
		const char* name,
		GLint position);

	void addFrameBuffer();

	void setRenderableUniformParameter(ShaderUniformParameter uniform);

	void setMap(ShaderMap map);

	string readShaderCode(const char* file);

	void initializeGL();
	void paintGL();
	void clear();

	GeometryInfo* makeCube();
};