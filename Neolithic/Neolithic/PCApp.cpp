#include "PCApp.h"

PCApp::PCApp()
{
}

int PCApp::run()
{
	module::Perlin myModule;
	utils::NoiseMap heightMap;
	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(myModule);
	heightMapBuilder.SetDestNoiseMap(heightMap);
	heightMapBuilder.SetDestSize(256,256);
	heightMapBuilder.SetBounds(2.0,6.0,1.0,5.0);
	heightMapBuilder.Build();

	utils::RendererImage renderer;
	utils::Image image;
	renderer.SetSourceNoiseMap(heightMap);
	renderer.SetDestImage(image);
	renderer.ClearGradient();
	renderer.AddGradientPoint(-1.00,utils::Color(  0,  0,128,255)); // trench
	renderer.AddGradientPoint(-0.80,utils::Color(  0,  0,255,255)); // shelf
	renderer.AddGradientPoint(-0.20,utils::Color(  0,255,255,255)); // shore
	renderer.AddGradientPoint(-0.10,utils::Color(255,255,128,255)); // beach
	renderer.AddGradientPoint( 0.00,utils::Color(128,255,  0,255)); // plain
	renderer.AddGradientPoint( 0.60,utils::Color(  0,255,  0,255)); // forest
	renderer.AddGradientPoint( 0.80,utils::Color(128,128,128,255)); // mountain
	renderer.AddGradientPoint( 1.00,utils::Color(255,255,255,255)); // snowcap
	renderer.Render();

	utils::WriterBMP writer;
	writer.SetSourceImage(image);
	writer.SetDestFilename("test.bmp");
	writer.WriteDestFile();

	return 0;
}