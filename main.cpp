// nifRenderer Main

#include <QApplication>
#include "viewer.h"

// Main
int main(int argc,char* argv[]){
	QApplication app(argc, argv);
	srand(time(NULL));
	Viewer viewer;
	viewer.show();
	//Start the main loop
	return app.exec();
}
