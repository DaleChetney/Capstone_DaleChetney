#pragma once
#include <QtGui\qwidget>
#include <QtGui\QHboxlayout>
#include <QtGui\QVboxlayout>
#include <QtGui\qlabel>
#include "Renderer.h"
#include <ShapeGenerator.h>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtx\rotate_vector.hpp>
#include "Camera.h"
#include <Qt/qtimer.h>
#include <QtGui\qmouseevent>
#include <QtGui\qkeyevent>
#include "Human.h"
#include "Tribe.h"
#include "DebugElement.h"
#include "DebugButton.h"
#include "FastDelegate.h"
#include <CL\cl.h>
#include <CL\cl_gl.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

using Neumont::ShapeGenerator;
using Neumont::ShapeData;
using glm::mat4;
using glm::vec3;
using glm::rotate;
using glm::translate;
using glm::scale;
using glm::inverse;
using glm::dot;

class GraphicsWindow : public QWidget
{
	Q_OBJECT
	QHBoxLayout* mainLayout;
	QVBoxLayout* controlPanel;
	QVBoxLayout* infoPanel;

	DebugButton* landscapeButton;
	DebugButton* fertilityButton;
	DebugButton* densityButton;
	QLabel* labelTribes;
	QLabel* labelPop;
	QLabel* labelTribPop;
	QLabel* labelTribFood;
	QLabel* labelTribHarvest;
	
	Geometry* plane;
	Geometry* cube;

	float landAmp;
	float viewTribe;
	float soil;
	float lifespan;
	float birthrate;
	float exploration;

	Shader* landscapeShader;
	Shader* cityOverlay;
	Shader* simpleShader;

	Texture* worldTexture;
	Texture* heightTexture;
	Texture* fertilityTexture;
	Texture* cityTexture;
	Texture* densityTexture;

	RENDER* land;
	RENDER* city;
	RENDER* indicator;

	mat4 projection;
	mat4 cameraPosition;

	Camera cam;
	QTimer gameTimer;
	void setUpLevel();
	void mouseMoveEvent(QMouseEvent*);
	void mousePressEvent(QMouseEvent*);
	void setShapeUVs(ShapeData* shape);
	void checkErr(cl_int err, const char* name);

	cl_int err;
	cl_int numImageBuffers;
	cl_int numKernels;
	vector<cl_device_id> deviceIds;
	cl_program program;
	cl_command_queue queue;
	vector<cl_mem> imageBuffer;

	static const int MAX_TRIBES = 16;
	int currentTribes;
	Tribe tribes[MAX_TRIBES];
	cl_mem tribeBuf;
	
	static const int MAX_POPULATION = 350;
	int currentPopulation;
	Human population[MAX_POPULATION];
	cl_mem populationBuf;

	vector<cl_kernel> kernel;
	cl_context context;

	void setupCL();
	void initializeWorld();
	void runCL();
	void closeCL();
private slots:
	void gameUpdate();
	void gatherSliderData(){}
public:
	GraphicsWindow();
	void setShaders();
	void setTextures();
	void setGeomtries();
	void setRenders();
	void selectLandscape();
	void selectFertility();
	void selectDensity();
	void addButton(DebugButton* button, char* label,fastdelegate::FastDelegate0<>callback);
};