#include <Qt\qapplication.h>
#include "GraphicsWindow.h"
#include <ctime>
#include <mmsystem.h>
 static QApplication* app;

int main(int argc, char* argv[])
{
	app = new QApplication(argc,argv);
	GraphicsWindow game;
	return app->exec();
}