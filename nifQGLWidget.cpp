
#include <QGLWidget>
#include <QString>
#include <QtOpenGL>

#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>

#include "nifQGLWidget.h"

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

using namespace Niflib;
using namespace std;


nifQGLWidget::nifQGLWidget(QWidget* parent)
	:QGLWidget(parent)
{
	//Initial Camera Position at 0,10,0
	cameraLoc[0] = 0.0;
	cameraLoc[1] = 100.0;
	cameraLoc[2] = 0.0;
	
	light=1;      //  Lighting

	QPoint pos;
	th=0;         //  Azimuth of view angle
	ph=270;         //  Elevation of view angle
	axes=1;       //  Display axes
	asp=1;
	dim=100.0;
	fov=55;       //  Field of view (for perspective)
	
	// Light values
	lightdistance = 40; // Light distance
	inc = 10;  			// Ball increment
	smooth = 1;  		// Smooth/Flat shading
	local = 0;  		// Local Viewer Model
	emission = 0.00;  	// Emission intensity (%)
	ambient = 0.30;  	// Ambient intensity (%)
	diffuse = 1.00; 	// Diffuse intensity (%)
	specular = 0.00;	// Specular intensity (%)
	shininess = 0; 		// Shininess (power of two)
	shiny = 1;  		// Shininess (value)
	zh = 90;  			// Light azimuth
	ylight = 0;  		// Elevation of light
	lightRot = 1; 			//Light rotation toggle
	
	spawncounter = 0;
	mouse = 0; // Mouse Toggle
	
	//Set up the Timer event to start
	startTimer(0);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~ Param Setters ~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


void nifQGLWidget::setNifTransX(int index, int tShapeIndex, float value){
	//Just a pass into the nifFileController to update the value given. 
	nifFiles[index]->setTransXtShape(tShapeIndex, value);
}
void nifQGLWidget::setNifTransY(int index, int tShapeIndex, float value){
	nifFiles[index]->setTransYtShape(tShapeIndex, value);
}
void nifQGLWidget::setNifTransZ(int index, int tShapeIndex, float value){
	nifFiles[index]->setTransZtShape(tShapeIndex, value);
}

void nifQGLWidget::setNifRotAngle(int index, int tShapeIndex, float value){
	nifFiles[index]->setRotAngletShape(tShapeIndex, value);
}
void nifQGLWidget::setNifRotX(int index, int tShapeIndex, float value){
	nifFiles[index]->setRotXtShape(tShapeIndex, value);
}
void nifQGLWidget::setNifRotY(int index, int tShapeIndex, float value){
	nifFiles[index]->setRotYtShape(tShapeIndex, value);
}
void nifQGLWidget::setNifRotZ(int index, int tShapeIndex, float value){
	nifFiles[index]->setRotZtShape(tShapeIndex, value);
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~ Scene Events ~~~~~~~~*/
/*~~~~~~~~~~-~~~~~~~~~~~~~~~~~~~*/

//TODO: Look at implementing this using 'deleteNif' instead
void nifQGLWidget::resetScene(){
	//Reset View
	th = 0;
	ph = 270;
	dim = 100.0;
	fov = 55;
	
	//Reset Light
	lightdistance = 15; 
	inc = 10;  
	smooth = 1;  		
	local = 0;  		
	emission = 0.00;  	
	ambient = 0.70;  	
	diffuse = 1.00; 	
	specular = 0.00;	
	shininess = 0; 		
	shiny = 1;  		
	zh = 90;  			
	ylight = 0;  		
	lightRot = 1;
	light = 1;
	
	//Empty the nifFiles vector. Destructor should clear all associated textures to prevent a memory leak.
	for(unsigned int i = 0; i < nifFiles.size(); i++){
		delete nifFiles[i];
	}
	nifFiles.clear();
	//Clearing the vector will not reset the size of the vector, however. For that, we need to shrink the vector.

	emit clearedScene(); //Let the UI know the clearing of the window is finished.
}

void nifQGLWidget::initializeGL(){
	setMouseTracking(true);
}

void nifQGLWidget::resizeGL(int width, int height){
	asp = (width && height) ? width / (float)height : 1;
	glViewport(0,0,width,height);
	project();
}
void nifQGLWidget::paintGL(){
	const double len=2.0;  //  Length of axes
    //  Erase the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //  Enable Z-buffering in OpenGL
    glEnable(GL_DEPTH_TEST);
    //  Undo previous transformations
    glLoadIdentity();
    //  Perspective - set eye position
    glPushMatrix();
    if(ph%360 <= 270){
		gluLookAt(cameraLoc[0],cameraLoc[1],cameraLoc[2], cameraLoc[0]-(Sin(th)*Cos(ph)),cameraLoc[1]+Sin(ph),cameraLoc[2]+Cos(th)*Cos(ph),0,-1,0);
	}else{
		gluLookAt(cameraLoc[0],cameraLoc[1],cameraLoc[2], cameraLoc[0]-(Sin(th)*Cos(ph)),cameraLoc[1]+Sin(ph),cameraLoc[2]+Cos(th)*Cos(ph),0,1,0);
	}
    
        
    if (light)
    {
        //  Translate intensity to color vectors
        
        float Ambient[]   = {ambient ,ambient ,ambient ,1.0};
        float Diffuse[]   = {diffuse ,diffuse ,diffuse ,1.0};
        float Specular[]  = {specular,specular,specular,1.0};
        //  Light position
        float lightX = lightdistance*Cos(zh);
        float lightZ = lightdistance*Sin(zh);
        float Position[]  = {lightX,ylight,lightZ,1.0};
        //  Draw light position as ball (still no lighting here)
        glColor3f(1,1,1);
        ball(Position[0],Position[1],Position[2] , 1);
        
        //  OpenGL should normalize normal vectors
        glEnable(GL_NORMALIZE);
        //  Enable lighting
        glEnable(GL_LIGHTING);
        //  Location of viewer for specular calculations
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
        //  glColor sets ambient and diffuse color materials
        glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);
        //  Enable light 0
        glEnable(GL_LIGHT0);
        //  Set ambient, diffuse, specular components and position of light 0
        glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
        glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
        glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
        glLightfv(GL_LIGHT0,GL_POSITION,Position);
    }
    else
        glDisable(GL_LIGHTING);
    //  Draw scene
    for(unsigned int i = 0; i < nifFiles.size(); i++){
        glPushMatrix();
        
        nifFiles[i]->renderObjectTrishapes();
        glPopMatrix();
        
    }
    
    //  Draw axes - no lighting from here on

    glColor3f(1,1,1);
    if (axes)
    {
		glDisable(GL_TEXTURE_2D);
        glBegin(GL_LINES);
        glVertex3d(0.0,0.0,0.0);
        glVertex3d(len,0.0,0.0);
        glVertex3d(0.0,0.0,0.0);
        glVertex3d(0.0,len,0.0);
        glVertex3d(0.0,0.0,0.0);
        glVertex3d(0.0,0.0,len);
        glEnd();
    }
    glPopMatrix();
    

    
    //  Render the scene and make it visible
    glFlush();
    //glutSwapBuffers();
}

void nifQGLWidget::project(){
	//  Tell OpenGL we want to manipulate the projection matrix
    glMatrixMode(GL_PROJECTION);
    //  Undo previous transformations
    glLoadIdentity();
    gluPerspective(fov,asp,dim/16,16*dim);
    //  Switch to manipulating the model matrix
    glMatrixMode(GL_MODELVIEW);
}

void nifQGLWidget::mousePressEvent(QMouseEvent* e){
	mouse = true;
	pos = e->pos();
}
void nifQGLWidget::mouseReleaseEvent(QMouseEvent*){
	mouse = false;
}
void nifQGLWidget::mouseMoveEvent(QMouseEvent* e){
	if(mouse){
		QPoint d = e->pos() - pos;
		th = (th+d.x())%360;
		ph = (ph+d.y())%360;
		pos = e->pos();
		update();
	}
}
void nifQGLWidget::wheelEvent(QWheelEvent* e){
	if(e->delta()<0){
		cameraLoc[0] += 3*Sin(th)*Cos(ph);
		cameraLoc[1] -= 3*Sin(ph);
		cameraLoc[2] -= 3*Cos(th)*Cos(ph);
	}else{
		cameraLoc[0] -= 3*Sin(th)*Cos(ph);
		cameraLoc[1] += 3*Sin(ph);
		cameraLoc[2] += 3*Cos(th)*Cos(ph);
	}
	update();
}



//timerEvent is basically being used as an emulation of the glutIdleFunc().
void nifQGLWidget::timerEvent(QTimerEvent*){
	if(lightRot){
		double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
		zh = fmod(90*t, 360);
		update();
	}
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~ File Loading Events ~~~~~~~~*/
/*~~~~~~~~~~-~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void nifQGLWidget::generateRandomObject(){
	string filename;
	if(spawncounter%3 == 1){
		filename = "helmet.nif";
	}else if(spawncounter%3 == 2){
		filename = "silverplate01.nif";
	}else if(spawncounter%3 == 0){
		filename = "upperchair01.nif";
	}
	nifFiles.push_back(new nifFileController(filename, 1));
	spawncounter++; //Increment our iterator for the next object we spawn
	emit newObj(nifFiles.size()-1, filename);
	update();
} 

void nifQGLWidget::loadNifFile(){
	string filename = "";
	QString qFileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("NIF files (*.nif)"));
	filename = qFileName.toStdString();
	
	nifFiles.push_back(new nifFileController(filename, 0)); //create a new nifFileController with the object's default translation.
	emit newObj(nifFiles.size()-1, filename);
	update();
} 


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~ Misc. Helper Functions ~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

string nifQGLWidget::getInfoFromIndex(int index){
	return nifFiles[index]->getInfoString();
}

void nifQGLWidget::toggleBBoxAllButOne(unsigned int index){
	for(unsigned int i = 0; i < nifFiles.size(); i++){
		if(index == i){
			nifFiles[i]->setRenderingBox(1);
		}else{
			nifFiles[i]->setRenderingBox(0);
		}
	}
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~ Light Helper Functions ~~~~~~~~*/
/*~~~~~~~~~~-~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void nifQGLWidget::Vertex(double th,double ph) //Helper function for creating the light sphere
{
    double x = Sin(th)*Cos(ph);
    double y = Cos(th)*Cos(ph);
    double z =         Sin(ph);
    //  For a sphere at the origin, the position
    //  and normal vectors are the same
    glNormal3d(x,y,z);
    glVertex3d(x,y,z);
}

/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius (r)
 */
void nifQGLWidget::ball(double x,double y,double z,double r)
{
    int th,ph;
    float yellow[] = {1.0,1.0,1.0,1.0};
    float Emission[]  = {0.0,0.0,emission,1.0};
    //  Save transformation
    glPushMatrix();
    //  Offset, scale and rotate
    glTranslated(x,y,z);
    glScaled(r,r,r);
    //  White ball
    glColor3f(1,1,1);
    glMaterialf(GL_FRONT,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
    glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
    //  Bands of latitude
    for (ph=-90;ph<90;ph+=inc)
    {
        glBegin(GL_QUAD_STRIP);
        for (th=0;th<=360;th+=2*inc)
        {
            Vertex(th,ph);
            Vertex(th,ph+inc);
        }
        glEnd();
    }
    //  Undo transofrmations
    glPopMatrix();
}
