#include <GL\glew.h>
#include "GraphicsWindow.h"
#include "noiseutils.h"
#include "noise\noise.h"

GraphicsWindow::GraphicsWindow()
{
	glewInit();
	setMouseTracking(true);
	mainLayout = new QHBoxLayout();
	setLayout(mainLayout);
	RENDERER->setFixedHeight(1060);
	RENDERER->setFixedWidth(1400);

	controlPanel = new QVBoxLayout();
	controlPanel->addLayout(new DebugElement("Elevation Sharpness",&landAmp,-10.0f,0.0f,-5.0f));
	controlPanel->addLayout(new DebugElement("Global Soil Quality",&soil,0.0f,10.0f,1.0f));
	controlPanel->addLayout(new DebugElement("Average Lifespan",&lifespan,0.0f,100.0f,50.0f));
	controlPanel->addLayout(new DebugElement("Global Birthrate",&birthrate,0.0f,100.0f,5.0f));
	controlPanel->addLayout(new DebugElement("City Development Range",&exploration,0.0f,100.0f,50.0f));
	addButton(landscapeButton,"View Landscape",fastdelegate::MakeDelegate(this,&GraphicsWindow::selectLandscape));
	addButton(fertilityButton,"View Fertility",fastdelegate::MakeDelegate(this,&GraphicsWindow::selectFertility));
	addButton(densityButton,"View Foliage Density",fastdelegate::MakeDelegate(this,&GraphicsWindow::selectDensity));
	mainLayout->addLayout(controlPanel);

	mainLayout->addWidget(RENDERER);

	infoPanel = new QVBoxLayout();
	labelTribes = new QLabel();
	infoPanel->addWidget(labelTribes);
	labelPop = new QLabel();
	infoPanel->addWidget(labelPop);
	infoPanel->addLayout(new DebugElement("View Tribe: ",&viewTribe,0.0f,15.0f,0.0f));
	labelTribPop = new QLabel();
	infoPanel->addWidget(labelTribPop);
	labelTribHarvest = new QLabel();
	infoPanel->addWidget(labelTribHarvest);
	labelTribFood = new QLabel();
	infoPanel->addWidget(labelTribFood);
	mainLayout->addLayout(infoPanel);

	showFullScreen();

	projection = glm::perspective(45.0f, (float)RENDERER->width()/RENDERER->height(),0.01f,300.0f);
	cameraPosition = cam.getWorldtoVeiwMatrix();

	setUpLevel();
	setupCL();
	initializeWorld();
	

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
	cam.position = vec3(0.0f,75.0f,50.0f);
	cam.orientation = glm::normalize(vec3(0.0f,-1.0f,-0.5f));
	currentPopulation = 30;
	currentTribes = 5;
	srand(52341);
}

void GraphicsWindow::setShaders()
{
	landAmp = -5.0f;
	landscapeShader = RENDERER->createShaderInfo("../../../Middleware/ChetEngine/Shaders/LandscapeVert.glsl","../../../Middleware/ChetEngine/Shaders/LandscapeFrag.glsl");
	RENDERER->addShaderUniformParameter(landscapeShader,"wtvTransform",ParamType::PT_MAT4,&cameraPosition[0][0]);
	RENDERER->addShaderUniformParameter(landscapeShader,"vtpTransform",ParamType::PT_MAT4,&projection[0][0]);
	RENDERER->addShaderUniformParameter(landscapeShader,"amplitude",ParamType::PT_FLOAT,&landAmp);

	cityOverlay = RENDERER->createShaderInfo("../../../Middleware/ChetEngine/Shaders/LandscapeVert.glsl","CityOverlayFrag.glsl");
	RENDERER->addShaderUniformParameter(cityOverlay,"wtvTransform",ParamType::PT_MAT4,&cameraPosition[0][0]);
	RENDERER->addShaderUniformParameter(cityOverlay,"vtpTransform",ParamType::PT_MAT4,&projection[0][0]);
	RENDERER->addShaderUniformParameter(cityOverlay,"amplitude",ParamType::PT_FLOAT,&landAmp);

	simpleShader = RENDERER->createShaderInfo("../../../Middleware/ChetEngine/Shaders/DumbVertexShader.glsl","../../../Middleware/ChetEngine/Shaders/DumbFragmentShader.glsl");
	RENDERER->addShaderUniformParameter(simpleShader,"wtvTransform",ParamType::PT_MAT4,&cameraPosition[0][0]);
	RENDERER->addShaderUniformParameter(simpleShader,"vtpTransform",ParamType::PT_MAT4,&projection[0][0]);
}

void GraphicsWindow::setTextures()
{
	//module::RidgedMulti mountains;
	//mountains.SetOctaveCount(6);
	//mountains.SetFrequency(1.0f);
	//mountains.SetLacunarity(2.2f);
	//module::Billow hills;
	//hills.SetOctaveCount(6);
	//hills.SetFrequency(0.8f);
	//hills.SetPersistence(0.5f);
	//module::ScaleBias plains;
	//plains.SetSourceModule(0,hills);
	//plains.SetScale(0.5f);
	//plains.SetBias(0.0f);
	//
	//module::RidgedMulti landtype;
	//landtype.SetOctaveCount(4);
	//landtype.SetFrequency(0.5f);
	//module::Select landSelect;
	//landSelect.SetSourceModule(0,plains);
	//landSelect.SetSourceModule(1,mountains);
	//landSelect.SetControlModule(landtype);
	//landSelect.SetBounds(0.3f,10.0f);
	//landSelect.SetEdgeFalloff(0.35f);
	//module::ScaleBias land;
	//land.SetSourceModule(0,landSelect);
	//land.SetScale(0.5f);
	//land.SetBias(0.8f);
	//
	//module::ScaleBias ocean;
	//ocean.SetSourceModule(0,land);
	//ocean.SetScale(0.3f);
	//ocean.SetBias(-0.5f);
	//
	//module::Perlin coast;
	//coast.SetOctaveCount(5);
	//coast.SetFrequency(0.1f);
	//coast.SetPersistence(0.7f);
	//module::Select selector;
	//selector.SetSourceModule(0,ocean);
	//selector.SetSourceModule(1,land);
	//selector.SetControlModule(coast);
	//selector.SetBounds(0.5f,10.0f);
	//selector.SetEdgeFalloff(0.8f);
	//
	//module::Turbulence myModule;
	//myModule.SetSourceModule(0,selector);
	//myModule.SetFrequency(0.5f);
	//myModule.SetPower(0.35f);
	//
	//utils::NoiseMap heightMap;
	//utils::NoiseMapBuilderPlane heightMapBuilder;
	//heightMapBuilder.SetSourceModule(myModule);
	//heightMapBuilder.SetDestNoiseMap(heightMap);
	//heightMapBuilder.SetDestSize(1024,1024);
	//heightMapBuilder.SetBounds(5.0,10.0,5.0,10.0);
	//heightMapBuilder.Build();
	//
	//utils::RendererImage renderer;
	//utils::Image image;
	//renderer.SetSourceNoiseMap(heightMap);
	//renderer.SetDestImage(image);
	//renderer.ClearGradient();
	//
	////world
	//renderer.AddGradientPoint(-1.00,utils::Color(  0,  0,128,255)); // trench
	//renderer.AddGradientPoint(-0.50,utils::Color(  0,  0,255,255)); // shelf
	//renderer.AddGradientPoint( 0.12,utils::Color(  0,255,255,255)); // shore
	//renderer.AddGradientPoint( 0.14,utils::Color(254,255,200,255)); // beach
	//renderer.AddGradientPoint( 0.16,utils::Color(128,200, 50,255)); // plain
	//renderer.AddGradientPoint( 0.50,utils::Color( 50,144, 60,255)); // forest
	//renderer.AddGradientPoint( 0.90,utils::Color(128,128,128,255)); // mountain
	//renderer.AddGradientPoint( 1.19,utils::Color(180,180,180,255)); // uppermountain
	//renderer.AddGradientPoint( 1.20,utils::Color(254,255,255,255)); // snowcap
	////height
	//renderer.AddGradientPoint( 0.13,utils::Color(  0,0,0,255)); // sealevel
	//renderer.AddGradientPoint( 1.20,utils::Color(255,255,255,255)); // summit
	////fertility
	//renderer.AddGradientPoint( 0.14,utils::Color(0,0,0,255)); // beach
	//renderer.AddGradientPoint( 0.18,utils::Color(100,100,100,255)); // lowlands
	//renderer.AddGradientPoint( 0.28,utils::Color(170,170,170,255)); // lower plain limit
	//renderer.AddGradientPoint( 0.30,utils::Color(200,200,200,255)); // lower plain
	//renderer.AddGradientPoint( 0.34,utils::Color(185,185,185,255)); // upper plain
	//renderer.AddGradientPoint( 0.38,utils::Color(150,150,150,255)); // upper plain limit
	//renderer.AddGradientPoint( 0.50,utils::Color(160,160,160,255)); // highlands
	//renderer.AddGradientPoint( 0.90,utils::Color(0,0,0,255)); // mountain
	////density
	//renderer.AddGradientPoint( 0.14,utils::Color(0,0,0,255)); // beach
	//renderer.AddGradientPoint( 0.16,utils::Color(80,80,80,255)); // shrubland
	//renderer.AddGradientPoint( 0.29,utils::Color(100,100,100,255)); // lower plain limit
	//renderer.AddGradientPoint( 0.30,utils::Color(30,30,30,255)); // lower plain
	//renderer.AddGradientPoint( 0.41,utils::Color(30,30,30,255)); // upper plain
	//renderer.AddGradientPoint( 0.42,utils::Color(180,180,180,255)); // lower forest limit
	//renderer.AddGradientPoint( 0.61,utils::Color(160,160,160,255)); // upper forest limit
	//renderer.AddGradientPoint( 0.62,utils::Color(60,60,60,255)); // mountain plain
	//renderer.AddGradientPoint( 0.77,utils::Color(68,68,68,255)); // mountain plain
	//renderer.AddGradientPoint( 0.78,utils::Color(120,120,120,255)); // mountain plain limit
	//renderer.AddGradientPoint( 0.85,utils::Color(100,100,100,255)); // tree line
	//renderer.AddGradientPoint( 0.87,utils::Color(0,0,0,255)); // mountian
	//
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
	//writer.SetDestFilename("density.bmp");
	//writer.WriteDestFile();

	worldTexture = RENDERER->addTexture("world.bmp");
	heightTexture = RENDERER->addTexture("height.bmp");
	fertilityTexture = RENDERER->addTexture("fertility.bmp");
	densityTexture = RENDERER->addTexture("density.bmp");
	cityTexture = RENDERER->addTexture("city.bmp");
}

void GraphicsWindow::setGeomtries()
{
	plane = RENDERER->addGeometry(Neumont::ShapeGenerator::makePlane(100));
	RENDERER->addShaderStreamedParameter(plane,0,ParamType::PT_VEC3,Neumont::Vertex::POSITION_OFFSET,Neumont::Vertex::STRIDE);
	RENDERER->addShaderStreamedParameter(plane,1,ParamType::PT_VEC2,Neumont::Vertex::UV_OFFSET,Neumont::Vertex::STRIDE);

	cube = RENDERER->makeCube();
	RENDERER->addShaderStreamedParameter(cube,0,ParamType::PT_VEC3,Renderer::POSITION_OFFSET,Renderer::TANGENT_STRIDE);
	RENDERER->addShaderStreamedParameter(cube,1,ParamType::PT_VEC3,Renderer::UV_OFFSET,Renderer::TANGENT_STRIDE);
}

void GraphicsWindow::setRenders()
{
	land = RENDERER->addRenderable(plane,rotate(-180.0f,vec3(1,0,0)),mat4(),landscapeShader,worldTexture);
	city = RENDERER->addRenderable(plane,translate(vec3(0.0f,0.1f,0.0f))*rotate(-180.0f,vec3(1,0,0)),mat4(),cityOverlay,cityTexture);
	RENDERER->addRenderableMap(land,"heightMap",heightTexture->textureID);
	RENDERER->addRenderableMap(city,"heightMap",heightTexture->textureID);
	indicator = RENDERER->addRenderable(cube,mat4(),mat4(),simpleShader);
}

void GraphicsWindow::mouseMoveEvent(QMouseEvent* e)
{
	
}

void GraphicsWindow::gameUpdate()
{	
	runCL();

	labelTribes->setText(QString("Number of Tribes: ") + std::to_string(currentTribes).c_str());
	labelPop->setText(QString("Population: ") + std::to_string(currentPopulation).c_str());
	int tribeViewed = (int)viewTribe;
	if(tribeViewed<currentTribes)
	{
		labelTribPop->setText(QString("Tribe Population: ") + std::to_string(tribes[tribeViewed].population).c_str());
		labelTribFood->setText(QString("Tribe Food: ") + std::to_string(tribes[tribeViewed].storedFood).c_str());
		labelTribHarvest->setText(QString("Annual Tribe Harvest: ") + std::to_string(tribes[tribeViewed].annualHarvest).c_str());
		indicator->whereMatrix = translate(vec3(tribes[tribeViewed].center.s[0]/10.24f-50.0f,1.0f,tribes[tribeViewed].center.s[1]/-10.24f+50.0f))*scale(0.5f,0.5f,0.5f);
	}
	QPoint point = mapFromGlobal(QCursor::pos());
	cam.mouseUpdate(vec2(point.x(),point.y()));

	cam.update();
	cameraPosition = cam.getWorldtoVeiwMatrix();
	if(GetAsyncKeyState(VK_ESCAPE))
	{
		closeCL();
		exit(0);
	}
	else RENDERER->repaint();
}

void GraphicsWindow::selectLandscape()
{
	land->maps[0].mapID = worldTexture->textureID;
}

void GraphicsWindow::selectFertility()
{
	land->maps[0].mapID = fertilityTexture->textureID;
}

void GraphicsWindow::selectDensity()
{
	land->maps[0].mapID = densityTexture->textureID;
}

void GraphicsWindow::addButton(DebugButton* button, char* label,fastdelegate::FastDelegate0<>callback)
{
	button = new DebugButton();
	controlPanel->addWidget(button->button = new QPushButton(label));
	QObject::connect(button->button,  SIGNAL(clicked()), button, SLOT(buttonClicked()));
	button->delegate = callback;
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
	err = CL_SUCCESS;
	cl_uint platformIdCount = 1;

	vector<cl_platform_id> platformIds(platformIdCount);
	clGetPlatformIDs(platformIdCount,platformIds.data(), nullptr);

	checkErr((platformIds.size()!=0)? CL_SUCCESS : -1, "no platforms found");

	//-------------------Devices-----------------
	cl_uint deviceIdCount = 1;

	deviceIds = vector<cl_device_id>(deviceIdCount);
	clGetDeviceIDs (platformIds[0], CL_DEVICE_TYPE_ALL, deviceIdCount,deviceIds.data(), nullptr);

	checkErr((deviceIds.size()!=0)? CL_SUCCESS : -1, "no devices found");

	//-------------------Contexts-----------------
	const cl_context_properties contextProperties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)platformIds[0],
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
		0,0,
	};

	context = clCreateContext(contextProperties,deviceIdCount,deviceIds.data(),nullptr,nullptr,&err);
	checkErr(err, "context");

	//-----------------------Program & Kernel-------------------------
	numKernels = 7;
	const char* sources[7];
	size_t lengths[7];
	string results[7];
	const char* filenames[7] = 
	{
		"kernel_EnhanceLandscape.cl",
		"kernel_Populate.cl",
		"kernel_Settle.cl",
		"kernel_FindFarmland.cl",
		"kernel_DrawRoads.cl",
		"kernel_BuildBuildings.cl",
		"kernel_Reproduce.cl",
	};

	for(int i=0; i<numKernels; i++)
	{
		std::ifstream in(filenames[i]);
		results[i] = string((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
		sources[i] = results[i].data();
		lengths[i] = results[i].size();
	}

	program = clCreateProgramWithSource(context,numKernels,sources,lengths,&err);
	checkErr(err, "program");

	checkErr(
		clBuildProgram (program,deviceIdCount,deviceIds.data(),"-D CAMP=780,650",nullptr,nullptr),
		"build failed"
	);

	kernel = vector<cl_kernel>(7);
	kernel[0] = clCreateKernel (program,"Enhance", &err);
	kernel[1] = clCreateKernel (program,"Settle", &err);
	kernel[2] = clCreateKernel (program,"Populate", &err);
	kernel[3] = clCreateKernel (program,"FindGoodLand", &err);
	kernel[4] = clCreateKernel (program,"DrawRoads", &err);
	kernel[5] = clCreateKernel (program,"BuildBuildings", &err);
	kernel[6] = clCreateKernel (program,"Reproduce",&err);
	checkErr(err, "kernel");

	//--------------------------Buffers---------------------------------
	numImageBuffers = 5;
	imageBuffer = vector<cl_mem>(numImageBuffers);
	imageBuffer[0] = clCreateFromGLTexture2D (context,CL_MEM_READ_WRITE,GL_TEXTURE_2D,0,worldTexture->textureID,&err);
	imageBuffer[1] = clCreateFromGLTexture2D (context,CL_MEM_READ_WRITE,GL_TEXTURE_2D,0,heightTexture->textureID,&err);
	imageBuffer[2] = clCreateFromGLTexture2D (context,CL_MEM_READ_WRITE,GL_TEXTURE_2D,0,fertilityTexture->textureID,&err);
	imageBuffer[3] = clCreateFromGLTexture2D (context,CL_MEM_READ_WRITE,GL_TEXTURE_2D,0,densityTexture->textureID,&err);
	imageBuffer[4] = clCreateFromGLTexture2D (context,CL_MEM_READ_WRITE,GL_TEXTURE_2D,0,cityTexture->textureID,&err);
	populationBuf = clCreateBuffer(context,CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,sizeof(Human)*MAX_POPULATION,population,&err);
	tribeBuf = clCreateBuffer(context,CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,sizeof(Tribe)*MAX_TRIBES,tribes,&err);
	checkErr(err,"buffer");

	//---------------------------Command Queue-------------------------------
	queue = clCreateCommandQueue (context, deviceIds[0], 0, &err);
	checkErr(err,"Queue");
}

void GraphicsWindow::initializeWorld()
{
	glFinish();
	clEnqueueAcquireGLObjects(queue, numImageBuffers, imageBuffer.data(), 0, 0,nullptr); 
	
	int seed = rand();
	clSetKernelArg(kernel[0],0,sizeof(imageBuffer[0]),&imageBuffer[0]);
	clSetKernelArg(kernel[0],1,sizeof(imageBuffer[1]),&imageBuffer[1]);
	clSetKernelArg(kernel[0],2,sizeof(imageBuffer[2]),&imageBuffer[2]);
	clSetKernelArg(kernel[0],3,sizeof(imageBuffer[3]),&imageBuffer[3]);
	clSetKernelArg(kernel[0],4,sizeof(imageBuffer[0]),&imageBuffer[0]);

	clSetKernelArg(kernel[1],0,sizeof(imageBuffer[1]),&imageBuffer[1]);
	clSetKernelArg(kernel[1],1,sizeof(imageBuffer[2]),&imageBuffer[2]);
	clSetKernelArg(kernel[1],2,sizeof(imageBuffer[3]),&imageBuffer[3]);
	clSetKernelArg(kernel[1],3,sizeof(imageBuffer[4]),&imageBuffer[4]);
	clSetKernelArg(kernel[1],4,sizeof(tribeBuf),&tribeBuf);
	clSetKernelArg(kernel[1],5,sizeof(int),&seed);

	clSetKernelArg(kernel[2],0,sizeof(populationBuf),&populationBuf);
	clSetKernelArg(kernel[2],1,sizeof(tribeBuf),&tribeBuf);
	clSetKernelArg(kernel[2],2,sizeof(int),&seed);

	size_t TribesSize[1] = {currentTribes};
	size_t PopSize[1] = {currentPopulation};
	size_t ImageSize[2] = {1024, 1024};
	size_t range[2] = {100, 100};

	checkErr(
		clEnqueueNDRangeKernel(queue,kernel[0],2,0,ImageSize,0,0,nullptr,nullptr),
		"command not enqueued"
	);
	clFinish(queue);

	for(int i=0; i<currentTribes; i++)
	{
		seed = rand();
		clSetKernelArg(kernel[1],5,sizeof(int),&seed);
		clSetKernelArg(kernel[1],6,sizeof(int),&i);
		clFinish(queue);
		checkErr(
			clEnqueueNDRangeKernel(queue,kernel[1],2,0,ImageSize,0,0,nullptr,nullptr),
			"Settling"
		);
		clFinish(queue);

		checkErr(
			clEnqueueReadBuffer(queue,tribeBuf,CL_TRUE,0,sizeof(tribes),tribes,0,nullptr,nullptr),
			"read pop buffer"
		);
		clFinish(queue);
		if(tribes[i].center.s[0]<0||tribes[i].center.s[1]<0) i--;
	}

	checkErr(
		clEnqueueReleaseGLObjects(queue,numImageBuffers,imageBuffer.data(),0,0,nullptr),
		"command return failed"
	);

	

	for(int i=0; i<currentPopulation; i++)
	{
		population[i].tribe = rand()%currentTribes;
		population[i].destination.s[0] = 0;
		population[i].destination.s[1] = 0;
		population[i].previousPosition.s[0] = 0;
		population[i].previousPosition.s[1] = 0;
		population[i].homePosition.s[0] = tribes[population[i].tribe].center.s[0] + rand()%20-10;
		population[i].homePosition.s[1] = tribes[population[i].tribe].center.s[1] + rand()%20-10;
		population[i].position.s[0] = population[i].homePosition.s[0];
		population[i].position.s[1] = population[i].homePosition.s[1];
		population[i].age = rand()%50;
		tribes[population[i].tribe].population++;
	}

	checkErr(
		clEnqueueWriteBuffer(queue,populationBuf,CL_TRUE,0,sizeof(population),population,0,nullptr,nullptr),
		"write pop buffer"
	);
	clFinish(queue);

	checkErr(
		clEnqueueWriteBuffer(queue,tribeBuf,CL_TRUE,0,sizeof(tribes),tribes,0,nullptr,nullptr),
		"write trib buffer"
	);
	clFinish(queue);
}

void GraphicsWindow::runCL()
{
	glFinish();
	clEnqueueAcquireGLObjects(queue, numImageBuffers, imageBuffer.data(), 0, 0,nullptr); 
	
	int seed = rand();

	clSetKernelArg(kernel[1],0,sizeof(imageBuffer[1]),&imageBuffer[1]);
	clSetKernelArg(kernel[1],1,sizeof(imageBuffer[2]),&imageBuffer[2]);
	clSetKernelArg(kernel[1],2,sizeof(imageBuffer[3]),&imageBuffer[3]);
	clSetKernelArg(kernel[1],3,sizeof(imageBuffer[4]),&imageBuffer[4]);
	clSetKernelArg(kernel[1],4,sizeof(tribeBuf),&tribeBuf);

	clSetKernelArg(kernel[3],0,sizeof(imageBuffer[1]),&imageBuffer[1]);
	clSetKernelArg(kernel[3],1,sizeof(imageBuffer[2]),&imageBuffer[2]);
	clSetKernelArg(kernel[3],2,sizeof(imageBuffer[3]),&imageBuffer[3]);
	clSetKernelArg(kernel[3],3,sizeof(imageBuffer[4]),&imageBuffer[4]);
	clSetKernelArg(kernel[3],4,sizeof(populationBuf),&populationBuf);
	clSetKernelArg(kernel[3],5,sizeof(tribeBuf),&tribeBuf);
	clSetKernelArg(kernel[3],6,sizeof(int),&seed);
	clSetKernelArg(kernel[3],8,sizeof(float),&soil);
	int devRange = (int)exploration;
	clSetKernelArg(kernel[3],9,sizeof(int),&devRange);
	
	clSetKernelArg(kernel[4],0,sizeof(imageBuffer[4]),&imageBuffer[4]);
	clSetKernelArg(kernel[4],1,sizeof(imageBuffer[3]),&imageBuffer[3]);
	clSetKernelArg(kernel[4],2,sizeof(imageBuffer[1]),&imageBuffer[1]);
	clSetKernelArg(kernel[4],3,sizeof(imageBuffer[4]),&imageBuffer[4]);
	clSetKernelArg(kernel[4],4,sizeof(populationBuf),&populationBuf);
	clSetKernelArg(kernel[4],5,sizeof(tribeBuf),&tribeBuf);
	clSetKernelArg(kernel[4],6,sizeof(int),&seed);
	
	clSetKernelArg(kernel[5],0,sizeof(imageBuffer[4]),&imageBuffer[4]);
	clSetKernelArg(kernel[5],1,sizeof(imageBuffer[4]),&imageBuffer[4]);
	clSetKernelArg(kernel[5],2,sizeof(int),&seed);
	
	size_t D1WorkSize[1] = {currentPopulation};
	size_t D2WorkSize[2] = {1024, 1024};
	size_t range[2] = {100, 100};

	if(currentPopulation>0)
	{
		
		for(int i=0; i<currentPopulation; i++)
		{
			if(population[i].age>14&&population[i].age<40&&population[i].destination.s[0]==0)
			{
				clSetKernelArg(kernel[3],7,sizeof(int),&i);
				clFinish(queue);
				checkErr(
					clEnqueueNDRangeKernel(queue,kernel[3],2,0,D2WorkSize,0,0,nullptr,nullptr),
					"Farming"
				);
				clFinish(queue);
			}
		}

		checkErr(
			clEnqueueNDRangeKernel(queue,kernel[4],1,0,D1WorkSize,0,0,nullptr,nullptr),
			"Pathing"
		);
		clFinish(queue);

	}
	
		checkErr(
			clEnqueueNDRangeKernel(queue,kernel[5],2,0,D2WorkSize,0,0,nullptr,nullptr),
			"Building"
		);
		clFinish(queue);

	if(currentPopulation>0)
	{
		checkErr(
			clEnqueueReadBuffer(queue,tribeBuf,CL_TRUE,0,sizeof(tribes),tribes,0,nullptr,nullptr),
			"read trib buffer"
		);
		clFinish(queue);

		checkErr(
			clEnqueueReadBuffer(queue,populationBuf,CL_TRUE,0,sizeof(population),population,0,nullptr,nullptr),
			"read pop buffer"
		);
		clFinish(queue);
	

		for(int i=0; i<currentTribes; i++)
		{
			tribes[i].storedFood+=tribes[i].annualHarvest;
		}

		for(int i=0; i<currentPopulation; i++)
		{
			population[i].age++;
			if(tribes[population[i].tribe].storedFood>0)tribes[population[i].tribe].storedFood--;
			if(population[i].age>15&&population[i].age<40&&rand()%100<=birthrate&&currentPopulation<MAX_POPULATION)
			{
				currentPopulation++;
				population[currentPopulation-1].destination.s[0] = 0;
				population[currentPopulation-1].destination.s[1] = 0;
				population[currentPopulation-1].previousPosition.s[0] = 0;
				population[currentPopulation-1].previousPosition.s[1] = 0;
				population[currentPopulation-1].homePosition.s[0] = population[i].position.s[0]+rand()%3;
				population[currentPopulation-1].homePosition.s[1] = population[i].position.s[1]+rand()%3;
				population[currentPopulation-1].position.s[0] = population[currentPopulation-1].homePosition.s[0];
				population[currentPopulation-1].position.s[1] = population[currentPopulation-1].homePosition.s[1];
				population[currentPopulation-1].age = 0;
				population[currentPopulation-1].tribe = population[i].tribe;
				tribes[population[i].tribe].population++;
			}
			if(population[i].age>=lifespan)
			{
				tribes[population[i].tribe].population--;
				if(tribes[population[i].tribe].population<=0)
				{
					tribes[population[i].tribe] = tribes[currentTribes-1];
					tribes[currentTribes-1].population=0;
					tribes[currentTribes-1].center.s[0]=0;
					tribes[currentTribes-1].center.s[1]=0;
					tribes[currentTribes-1].range=0;
					tribes[currentTribes-1].annualHarvest=0;
					tribes[currentTribes-1].storedFood=0;
					for(int j=0; j<currentPopulation; j++)
					{
						if(population[j].tribe==currentTribes-1)
						{
							population[j].tribe = population[i].tribe;
						}
					}
					currentTribes--;
				}
				population[i] = population[currentPopulation-1];
				population[currentPopulation-1].destination.s[0] = 0;
				population[currentPopulation-1].destination.s[1] = 0;
				population[currentPopulation-1].previousPosition.s[0] = 0;
				population[currentPopulation-1].previousPosition.s[1] = 0;
				population[currentPopulation-1].tribe = 0;
				population[currentPopulation-1].homePosition.s[0] = 0;
				population[currentPopulation-1].homePosition.s[1] = 0;
				population[currentPopulation-1].position.s[0] = 0;
				population[currentPopulation-1].position.s[1] = 0;
				population[currentPopulation-1].age = 0;
				currentPopulation--;
			}
		}
		
		checkErr(
			clEnqueueWriteBuffer(queue,tribeBuf,CL_TRUE,0,sizeof(tribes),tribes,0,nullptr,nullptr),
			"write trib buffer"
		);
		clFinish(queue);
		

		if(rand()%100==20&&currentTribes<MAX_TRIBES)
		{
			seed = rand();
			clSetKernelArg(kernel[1],5,sizeof(int),&seed);
			clSetKernelArg(kernel[1],6,sizeof(int),&currentTribes);
			clFinish(queue);
			checkErr(
				clEnqueueNDRangeKernel(queue,kernel[1],2,0,D2WorkSize,0,0,nullptr,nullptr),
				"Settling"
			);
			clFinish(queue);
		
			checkErr(
				clEnqueueReadBuffer(queue,tribeBuf,CL_TRUE,0,sizeof(tribes),tribes,0,nullptr,nullptr),
				"read pop buffer"
			);
			clFinish(queue);
			if(tribes[currentTribes].center.s[0]>0&&tribes[currentTribes].center.s[1]>0&&MAX_POPULATION-currentPopulation>5)
			{
				tribes[currentTribes].population=rand()%5;
				for(int i=currentPopulation; i<tribes[currentTribes].population+currentPopulation; i++)
				{
					population[i].tribe = currentTribes;
					population[i].destination.s[0] = 0;
					population[i].destination.s[1] = 0;
					population[i].previousPosition.s[0] = 0;
					population[i].previousPosition.s[1] = 0;
					population[i].homePosition.s[0] = tribes[population[i].tribe].center.s[0] + rand()%20-10;
					population[i].homePosition.s[1] = tribes[population[i].tribe].center.s[1] + rand()%20-10;
					population[i].position.s[0] = population[i].homePosition.s[0];
					population[i].position.s[1] = population[i].homePosition.s[1];
					population[i].age = rand()%50;
				}
				currentPopulation+=tribes[currentTribes].population;
				currentTribes++;
			}
		}
		checkErr(
			clEnqueueWriteBuffer(queue,populationBuf,CL_TRUE,0,sizeof(population),population,0,nullptr,nullptr),
			"write pop buffer"
		);
		clFinish(queue);

		checkErr(
			clEnqueueReleaseGLObjects(queue,numImageBuffers,imageBuffer.data(),0,0,nullptr),
			"command return failed"
		);
		clFinish(queue);
	}
}

void GraphicsWindow::closeCL()
{
	clReleaseCommandQueue(queue);
	for(int i=0; i< numImageBuffers; i++)
	{
		clReleaseMemObject(imageBuffer[i]);
	}
	clReleaseMemObject(populationBuf);
	for(int i=0; i< numKernels; i++)
	{
		clReleaseKernel(kernel[i]);
	}
	clReleaseProgram(program);
	clReleaseContext(context);
}