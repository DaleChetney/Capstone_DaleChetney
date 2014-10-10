#include <CL\cl.h>
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

const string hw("Hello World\n");

inline void checkErr(cl_int err, const char* name)
{
	if (err != CL_SUCCESS)
	{
		cerr << "ERROR: " << name << " (" << err << ")" << endl;
		exit(EXIT_FAILURE);
	}
}

struct Image
{
	vector<char> pixel;
	int width, height;
};

Image LoadImage (const char* path)
{
	std::ifstream in (path, std::ios::binary);
	string s;
	in >> s;

	if(s!="P6")
	{
		cerr << "file not found or invalid" << endl;
		exit(1);
	}

	//read past metadata
	for(;;)
	{
		getline(in,s);
		if(s.empty())continue;
		if(s[0]!='#')break;
	}

	std::stringstream str(s);
	int width, height, maxColor;
	str >> width >> height;
	in >> maxColor;

	if(maxColor!=255)
	{
		cerr << "uhh bad colors" << endl;
		exit(1);
	}

	{
		string tmp;
		getline(in,tmp);
	}

	vector<char>data(width*height*3);
	in.read(reinterpret_cast<char*>(data.data()),data.size());

	const Image img = {data,width,height};
	return img;
}

void SaveImage(const Image& img, const char* path)
{
	std::ofstream out(path, std::ios::binary);

	out<<"P6\n";
	out<<img.width<<" "<<img.height<<"\n";
	out<<"255\n";
	out.write(img.pixel.data(),img.pixel.size());
}

Image RGBtoRGBA(const Image& input)
{
	Image result;
	result.width = input.width;
	result.height = input.height;

	for(size_t i=0; i<input.pixel.size(); i+=3)
	{
		result.pixel.push_back(input.pixel[i+0]);
		result.pixel.push_back(input.pixel[i+1]);
		result.pixel.push_back(input.pixel[i+2]);
		result.pixel.push_back(0);
	}

	return result;
}

Image RGBAtoRGB(const Image& input)
{
	Image result;
	result.width = input.width;
	result.height = input.height;

	for(size_t i=0; i<input.pixel.size(); i+=4)
	{
		result.pixel.push_back(input.pixel[i+0]);
		result.pixel.push_back(input.pixel[i+1]);
		result.pixel.push_back(input.pixel[i+2]);
	}

	return result;
}

void fmain()
{
	//-------------------Platform--------------
	
	//get number of platforms
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
	vector<cl_device_id> deviceIds(deviceIdCount);
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
		CL_CONTEXT_PLATFORM, // property 1 name, in this case the target platform
		reinterpret_cast<cl_context_properties>(platformIds[0]), // property 1 value
		0, // property 2 name, 0 means end of properties list
		0, // property 2 value
	};

	cl_int err = CL_SUCCESS;

	// create context
	cl_context context = clCreateContext(
		contextProperties, // properties
		deviceIdCount, // num devices
		deviceIds.data(), // list of devices
		nullptr, // notification callback
		nullptr, // notification output
		&err // error output
	);

	checkErr(err, "context");

	cout << "context created" << endl;

	//-----------------------Program & Kernel-------------------------
	
	//gaussian blur filter
	float filter[] =
	{
		1,2,1,
		2,4,2,
		1,2,1,
	};

	//normalize filter
	for(int i=0; i<9; i++)
	{
		filter[i]/=16.0f;
	}

	//load kernel
	std::ifstream in("HelloWorld_Kernel.cl");
	string result((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());

	//create source strings
	size_t lengths[1] = {result.size()};
	const char* sources[1] = {result.data()};

	//create programs
	cl_program program = clCreateProgramWithSource(
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
			"-D FILTER_SIZE=1", //build options
			nullptr, //notification callback
			nullptr //notification output
		),

		"build failed"
	);

	//kernel creation
	cl_kernel kernel = clCreateKernel (program, "helloworld", &err);

	checkErr(err, "kernel");

	//------------------------Load Image--------------------------------

	const auto image = RGBtoRGBA(LoadImage("test.ppm"));

	static const cl_image_format format = {CL_RGBA, CL_UNORM_INT8 };

	//--------------------------Buffers---------------------------------

	//reserve memory for input
	cl_mem inputImage = clCreateImage2D (
		context, //context
		//flags
		CL_MEM_READ_WRITE |//kernel will read a[] but not write to it
		CL_MEM_COPY_HOST_PTR,//since host pointer isn't null copy it
		&format, //format
		image.width, //width
		image.height, //height
		0, // row pitch, either 0 or >= width * byte size if host pointer is not null
		const_cast<char*>(image.pixel.data()), // host pointer
		&err // error report
	);

	checkErr(err,"in buff");

	//reserve memory for output
	cl_mem outputImage = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, image.width, image.height, 0, nullptr, &err);
	
	checkErr(err,"out buff");

	// filterBuffer
	cl_mem filterBuffer = clCreateBuffer(
		context, //context
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, //flags
		sizeof(float) * 9, //buffersize
		filter, //host pointer
		&err // error code
	);

	checkErr(err,"filter buff");

	//---------------------------Command Queue-------------------------------
	cl_command_queue queue = clCreateCommandQueue (context, deviceIds[0], 0, &err);
	checkErr(err,"Queue");

	//---------------------------Execution-----------------------------------

	// send buffer data in to kernel
	clSetKernelArg(
		kernel, // kernel
		0, // argument index
		sizeof(cl_mem), // argument size
		&inputImage // argument value
	);//float* in
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &filterBuffer);//float* out
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &outputImage);//float a

	//set dimensions
	size_t offset[3] = {0};
	size_t globalWorkSize[3] = {
		image.width, //x
		image.height, //y
		1 //z
	};

	checkErr(

		//put an execution command on the command queue
		clEnqueueNDRangeKernel(
			queue, // command queue
			kernel, // kernel
			2, //dimensions
			offset, //global work offset, it will always be null
			globalWorkSize, //work dimensions
			nullptr, // work group size
			0, // num events in event wait list
			nullptr, //event wait list, events that need to happen before executing
			nullptr // execution event
		),

		"command not enqueued"
	);

	// clear result image
	Image resultImage = image;
	std::fill(resultImage.pixel.begin(), resultImage.pixel.end(), 0);

	size_t imageOffset [3] = {0};
	size_t resultSize [3] = {resultImage.width,resultImage.height,1};

	checkErr(

		//put a read command on the command queue
		clEnqueueReadImage (
			queue, // command queue
			outputImage, // image
			CL_TRUE, //is blocking, if true the command does not return until the data is copied back into the buffer
			imageOffset, // read offset
			resultSize, //size of read
			0, // row pitch
			0, // slice pitch, 0 unless 3D
			resultImage.pixel.data(), // output position
			0, //num wait events
			nullptr, // wait events
			nullptr// read event
		),

		"command return failed"
	);

	SaveImage(RGBAtoRGB(resultImage),"blurredTest.ppm");

	clReleaseCommandQueue(queue);
	clReleaseMemObject(outputImage);
	clReleaseMemObject(filterBuffer);
	clReleaseMemObject(inputImage);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseContext(context);
}