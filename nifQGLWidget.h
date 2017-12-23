#include <QGLWidget>
#include <QString>

#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>

#include "nifRenderer.h"

//DDSLoader
#include "DDSLoader.h"

//niflib includes
#include "niflib/include/niflib.h"
#include "niflib/include/obj/NiNode.h"
#include "niflib/include/obj/NiSkinInstance.h"
#include "niflib/include/obj/NiTriShape.h"
#include "niflib/include/obj/NiTriShapeData.h"
#include "niflib/include/obj/BSLightingShaderProperty.h"
#include "niflib/include/obj/BSShaderTextureSet.h"


#define Cos(th) cos(3.1415926/180*(th))
#define Sin(th) sin(3.1415926/180*(th))



#ifndef NIFQGLWID_H
#define NIFQGLWID_H

class nifQGLWidget : public QGLWidget 
{
Q_OBJECT
private:
	
	QPoint pos;
	
	int light;      //  Lighting toggle
	int lightRot;   //  Light rotation toggle
	int th;         //  Azimuth of view angle
	int ph;         //  Elevation of view angle
	int axes;       //  Display axes
	double asp;		//  Aspect Ratio
	double dim;		//  Dimensions of world
	int fov;       	//  Field of view (for perspective)
	double cameraLoc[3]; //Location of camera eye in 3d space
	
	vector<nifFileController*> nifFiles; //Holds all nif files currently in the scene

	// Light values
	int one;  // Unit value
	float lightdistance;  // Light distance
	float lightCenter[3]; // Coordinates to translate center of light's rotation
	int inc;  // Ball increment
	int smooth;  // Smooth/Flat shading
	int local;  // Local Viewer Model
	float emission;  // Emission intensity (%)
	float ambient;  // Ambient intensity (%)
	float diffuse;  // Diffuse intensity (%)
	float specular;  // Specular intensity (%)
	int shininess;  // Shininess (power of two)
	float shiny;  // Shininess (value)
	int zh;  // Light azimuth
	float ylight;  // Elevation of light
	
	
	bool mouse; //Holds mouseclick state
	
	int spawncounter; //Used for spawning a random object. NOT an accurate counter of the number of objects in the scene; use nifFiles.size() for that.

	//Private Functions
	void project(); //Sets the projection
	void ball(double x,double y,double z,double r);
	void Vertex(double th,double ph);
public:
	nifQGLWidget(QWidget* parent = 0);
	string getInfoFromIndex(int index); //Returns a comma separated string of information on the specified nifFile.  
	QSize sizeHint() const {return QSize(600,600);}
	
	//The Following 7 setters are called by the setNifTransRot slot.
	void setNifTransX(int index, int tShapeIndex, float value); //set the value specified for the nifFile[index]
	void setNifTransY(int index, int tShapeIndex, float value);
	void setNifTransZ(int index, int tShapeIndex, float value);
	
	void setNifRotAngle(int index, int tShapeIndex, float value);
	void setNifRotX(int index, int tShapeIndex, float value);
	void setNifRotY(int index, int tShapeIndex, float value);
	void setNifRotZ(int index, int tShapeIndex, float value);
	void toggleBBoxAllButOne(unsigned int index);
public slots:

	void resetScene(void); //Resets the scene to default view, clears the nifFiles vector.
	void generateRandomObject(void); //Generates a random object from the internal list of included .nif objects
	void loadNifFile(); //Loads an object at the origin with its default translation.

	//void removeNifObject(int objectIndex);
	//TODO:
	/*
	void autoZoom(void); //Resets viewing angle, set world bounds to the edges of the scene based on current objects in nifFiles vector
	void autoZoomNifObject(int objectIndex); //Center camera on a specific object. Camera now rotates around this center point
	
	void setTexture(string _textureFileName, int objectIndex, int trishapeIndex); //Changes the texture for a single trishape within a nif object.
	void moveNifObject(int objectIndex, float _x,float _y, float _z); //Change the translation of a nif object
	*/
	
	
signals:
	void newObj(int index, string fileName); //Signal for updating the viewer's list of objects
	void clearedScene(void); //Empty the object list.
protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	//void timerEvent(QTimerEvent *event);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);
	void timerEvent(QTimerEvent*);
};

#endif /* NIFQGLWID_H */

