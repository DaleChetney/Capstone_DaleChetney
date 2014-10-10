#include <GL\glew.h>
#include "GraphicsWindow.h"
#include "noiseutils.h"
#include "noise\noise.h"

GraphicsWindow::GraphicsWindow()
{
	glewInit();
	setMouseTracking(true);
	mainLayout = new QVBoxLayout();
	setLayout(mainLayout);
	RENDERER->setFixedHeight(1024);
	RENDERER->setFixedWidth(1024);
	
	mainLayout->addWidget(RENDERER);

	showFullScreen();

	projection = glm::perspective(45.0f, (float)RENDERER->width()/RENDERER->height(),0.01f,300.0f);
	cameraPosition = cam.getWorldtoVeiwMatrix();

	setUpLevel();
	setupCL();
	

	connect(&gameTimer, SIGNAL(timeout()), this, SLOT(gameUpdate()));
	gameTimer.start(1000);
	gameTimer.start(0);
}

void GraphicsWindow::setUpLevel()
{
	setGeomtries();
	setShaders();
	setTextures();
	setRenders();
	cam.position = vec3(-0.5f,-0.5f,1.2f);
	cam.orientation = glm::normalize(vec3(0.0f,0.0f,-1.0f));
}

void GraphicsWindow::setShaders()
{
	textureOnlyShader = RENDERER->createShaderInfo("../../../Middleware/ChetEngine/Shaders/TextureVertexShader.glsl","../../../Middleware/ChetEngine/Shaders/TextureFragmentShader.glsl");
	RENDERER->addShaderUniformParameter(textureOnlyShader,"wtvTransform",ParamType::PT_MAT4,&cameraPosition[0][0]);
	RENDERER->addShaderUniformParameter(textureOnlyShader,"vtpTransform",ParamType::PT_MAT4,&projection[0][0]);
}

void GraphicsWindow::setTextures()
{
	//module::Perlin myModule;
	//utils::NoiseMap heightMap;
	//utils::NoiseMapBuilderPlane heightMapBuilder;
	//heightMapBuilder.SetSourceModule(myModule);
	//heightMapBuilder.SetDestNoiseMap(heightMap);
	//heightMapBuilder.SetDestSize(1028,1028);
	//heightMapBuilder.SetBounds(2.0,6.0,1.0,5.0);
	//heightMapBuilder.Build();
	//
	//utils::RendererImage renderer;
	//utils::Image image;
	//renderer.SetSourceNoiseMap(heightMap);
	//renderer.SetDestImage(image);
	//renderer.ClearGradient();
	//renderer.AddGradientPoint(-1.00,utils::Color(  0,  0,128,255)); // trench
	//renderer.AddGradientPoint(-0.50,utils::Color(  0,  0,255,255)); // shelf
	//renderer.AddGradientPoint( 0.19,utils::Color(  0,255,255,255)); // shore
	//renderer.AddGradientPoint( 0.20,utils::Color(254,255,200,255)); // beach
	//renderer.AddGradientPoint( 0.21,utils::Color(128,200, 50,255)); // plain
	//renderer.AddGradientPoint( 0.50,utils::Color(  0,144, 60,255)); // forest
	//renderer.AddGradientPoint( 0.90,utils::Color(128,128,128,255)); // mountain
	//renderer.AddGradientPoint( 1.19,utils::Color(180,180,180,255)); // uppermountain
	//renderer.AddGradientPoint( 1.20,utils::Color(254,255,255,255)); // snowcap
	//renderer.EnableLight();
	//renderer.SetLightContrast(3.0);
	//renderer.SetLightBrightness(2.0);
	//renderer.SetLightAzimuth(200);
	//renderer.SetLightElev(35);
	//renderer.SetLightColor(utils::Color(255,255,200,255));
	//renderer.Render();
	//
	//utils::WriterBMP writer;
	//writer.SetSourceImage(image);
	//writer.SetDestFilename("test.bmp");
	//writer.WriteDestFile();

	levelTexture = RENDERER->addTexture("test.bmp");
}

void GraphicsWindow::setGeomtries()
{
	plane = RENDERER->addGeometry(Neumont::ShapeGenerator::makePlane(2));
	RENDERER->addShaderStreamedParameter(plane,0,ParamType::PT_VEC3,Neumont::Vertex::POSITION_OFFSET,Neumont::Vertex::STRIDE);
	RENDERER->addShaderStreamedParameter(plane,1,ParamType::PT_VEC2,Neumont::Vertex::UV_OFFSET,Neumont::Vertex::STRIDE);
}

void GraphicsWindow::setRenders()
{
	RENDERER->addRenderable(plane,rotate(-90.0f,vec3(1,0,0)),mat4(),textureOnlyShader,levelTexture);
}

void GraphicsWindow::mouseMoveEvent(QMouseEvent* e)
{
	if(GetAsyncKeyState(VK_RBUTTON)) cam.mouseUpdate(vec2(e->x(),e->y()));
}

void GraphicsWindow::gameUpdate()
{	
	runCL();

	cam.update();
	cameraPosition = cam.getWorldtoVeiwMatrix();

	if(GetAsyncKeyState(VK_ESCAPE))
	{
		closeCL();
		exit(1);
	}
	RENDERER->repaint();
}

void GraphicsWindow::mousePressEvent(QMouseEvent* e)
{
	
}

void GraphicsWindow::checkErr(cl_int err, const char* name)
{
	if (err != CL_SUCCESS)
	{
		cerr << "ERROR: " << name << " (" << err << ")" << endl;
		if(err == CL_BUILD_PROGRAM_FAILURE)
		{
			size_t logSize;
			clGetProgramBuildInfo(program,deviceIds[0],CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize); 
			char* log = (char*)malloc(logSize);
			clGetProgramBuildInfo(program,deviceIds[0],CL_PROGRAM_BUILD_LOG, logSize, log, nullptr); 
			cerr << log << endl;
		}
		exit(EXIT_FAILURE);
	}
}

void GraphicsWindow::setupCL()
{
	cl_uint platformIdCount = 0;
	clGetPlatformIDs(
		0, //buffersize
		nullptr, // data output
		&platformIdCount //size output
	);

	cout << platformIdCount << " platforms found" << endl;

	//get platforms
	vector<cl_platform_id> platformIds(platformIdCount);
	clGetPlatformIDs(platformIdCount,platformIds.data(), nullptr);

	checkErr((platformIds.size()!=0)? CL_SUCCESS : -1, "no platforms found");

	//------------------Platform Name--------------

	//get name size
	size_t namesize = 0;
	clGetPlatformInfo(
		platformIds[0], // target platform
		CL_PLATFORM_NAME, // desired data
		0, //size
		nullptr, //data output
		&namesize //size output
	);

	//get name
	string platformName;
	platformName.resize(namesize);
	clGetPlatformInfo(platformIds[0],CL_PLATFORM_NAME, namesize,const_cast<char*>(platformName.data()),nullptr);

	cout << "Platform Name: " << platformName << endl;

	//-------------------Devices-----------------

	//get number of devices
	cl_uint deviceIdCount = 0;
	clGetDeviceIDs(
		platformIds[0], //target platform
		CL_DEVICE_TYPE_ALL, //type
		0, // buffer size
		nullptr, // data output 
		&deviceIdCount // size output
	);

	cout << deviceIdCount << " devices found" << endl;

	//get devices
	deviceIds = vector<cl_device_id>(deviceIdCount);
	clGetDeviceIDs (platformIds[0], CL_DEVICE_TYPE_ALL, deviceIdCount,deviceIds.data(), nullptr);

	checkErr((deviceIds.size()!=0)? CL_SUCCESS : -1, "no devices found");
	
	//------------------Device Name--------------

	//get name size
	size_t dnamesize = 0;
	clGetDeviceInfo(
		deviceIds[0], // target platform
		CL_DEVICE_NAME, // desired data
		0, //size
		nullptr, //data output
		&dnamesize //size output
	);

	//get name
	string deviceName;
	deviceName.resize(dnamesize);
	clGetDeviceInfo(deviceIds[0],CL_DEVICE_NAME, dnamesize,const_cast<char*>(deviceName.data()),nullptr);

	cout << "Device Name: " << deviceName << endl;

	//-------------------Contexts-----------------

	//set properties, this is what it will pretty much always be like
	const cl_context_properties contextProperties[] = 
	{
		CL_CONTEXT_PLATFORM, (cl_context_properties)platformIds[0],
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
		0,0,
	};

	err = CL_SUCCESS;

	// create context
	context = clCreateContext(
		contextProperties, // properties
		deviceIdCount, // num devices
		deviceIds.data(), // list of devices
		nullptr, // notification callback
		nullptr, // notification output
		&err // error output
	);

	checkErr(err, "context");

	//-----------------------Program & Kernel-------------------------

	//load kernel
	std::ifstream in("kernel_RedDots.cl");
	string result((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());

	//create source strings
	size_t lengths[1] = {result.size()};
	const char* sources[1] = {result.data()};

	//create programs
	program = clCreateProgramWithSource(
		context, // context
		1, // num filenames
		sources, // list of kernel filenames
		lengths, // list of the lengths of the filenames
		&err // error report
	);

	checkErr(err, "program");

	checkErr(

		//buildProgram
		clBuildProgram (
			program, //program
			deviceIdCount, //num devices
			deviceIds.data(), // list of devices
			nullptr, //build options
			nullptr, //notification callback
			nullptr //notification output
		),

		"build failed"
	);

	//kernel creation
	kernel = clCreateKernel (program, "RedDots", &err);

	checkErr(err, "kernel");

	//--------------------------Buffers---------------------------------

	//reserve memory for input
	imageBuffer = clCreateFromGLTexture2D (
		context,
		CL_MEM_READ_WRITE,
		GL_TEXTURE_2D,
		0,
		levelTexture->textureID,
		&err
	);

	checkErr(err,"buffer");

	//---------------------------Command Queue-------------------------------
	
	queue = clCreateCommandQueue (context, deviceIds[0], 0, &err);
	checkErr(err,"Queue");
}

void GraphicsWindow::runCL()
{
	//---------------------------Execution-----------------------------------

	glFinish();
	clEnqueueAcquireGLObjects(queue, 1, &imageBuffer, 0, 0,nullptr); 
	// send buffer data in to kernel
	clSetKernelArg(
		kernel, // kernel
		0, // argument index
		sizeof(imageBuffer), // argument size
		&imageBuffer // argument value
	);
	clSetKernelArg(kernel,1,sizeof(imageBuffer),&imageBuffer);

	//set dimensions
	size_t globalWorkSize[2] = {
		1024, //x
		1024, //y
	};

	checkErr(

		//put an execution command on the command queue
		clEnqueueNDRangeKernel(
			queue, // command queue
			kernel, // kernel
			2, //dimensions
			0, //global work offset, it will always be null
			globalWorkSize, //work dimensions
			nullptr, // work group size
			0, // num events in event wait list
			nullptr, //event wait list, events that need to happen before executing
			nullptr // execution event
		),

		"command not enqueued"
	);

	clFinish(queue);

	checkErr(

		//put a read command on the command queue
		clEnqueueReleaseGLObjects(
			queue,
			1,
			&imageBuffer,
			0,
			0,
			nullptr
		),

		"command return failed"
	);

	clFinish(queue);
}

void GraphicsWindow::closeCL()
{
	clReleaseCommandQueue(queue);
	clReleaseMemObject(imageBuffer);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseContext(context);
}