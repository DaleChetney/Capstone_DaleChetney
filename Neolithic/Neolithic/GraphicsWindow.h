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
	QVBoxLayout* mainLayout;
	
	Geometry* plane;

	Shader* textureOnlyShader;

	Texture* levelTexture;

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
	vector<cl_device_id> deviceIds;
	cl_program program;
	cl_command_queue queue;
	cl_mem imageBuffer;
	cl_kernel kernel;
	cl_context context;

	void setupCL();
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
};