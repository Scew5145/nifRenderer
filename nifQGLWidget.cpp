
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
	for(int i = 0; i < 9; i++){
		sFlags[i] = 1;
	}
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
	ambient = 0.50;  	// Ambient intensity (%)
	diffuse = 1.00; 	// Diffuse intensity (%)
	specular = 0.7;	// Specular intensity (%)
	shininess = 30; 		// Shininess (power of two)
	shiny = 20;  		// Shininess (value)
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
	dim = 120.0;
	fov = 55;
	
	//Reset Light
	lightdistance = 15; 
	inc = 10;  
	smooth = 1;  		
	local = 0;  		
	emission = 0.00;  	
	ambient = 0.30;  	
	diffuse = 1.00; 	
	specular = 0.70;	
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
	addShader(":/skyrimPipeline.vert",":/skyrimPipeline.frag", ",,,,,Tangent,Bitangent");
	glClearColor(0.5,0.5,0.5,1.0);
}

void nifQGLWidget::resizeGL(int width, int height){
	asp = (width && height) ? width / (float)height : 1;
	glViewport(0,0,width,height);
	project();
}
void nifQGLWidget::paintGL(){
	const double len=10.0;  //  Length of axes
    //  Erase the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //  Enable Z-buffering in OpenGL
    glEnable(GL_DEPTH_TEST);
    //  Undo previous transformations
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Enable Transperancy 
    //glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
    //  Perspective - set eye position
    int w = width()/devicePixelRatio();
    int h = height()/devicePixelRatio();
    float asp = w/(float)h;
    double zmin = dim/16;
    double zmax = dim*16;
    double ydim = zmin*tan(fov*M_PI/360);
    double xdim = ydim*asp;
    glFrustum(-xdim,+xdim,-ydim,+ydim,zmin,zmax);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0,0,-2*dim);
    glRotated(ph,1,0,0);
    glRotated(th,0,1,0);
    
    
        
    if (light)
    {
        //  Translate intensity to color vectors
        
        float Ambient[]   = {ambient ,ambient ,ambient ,1.0};
        float Diffuse[]   = {diffuse ,diffuse ,diffuse ,1.0};
        float Specular[]  = {specular,specular,specular,1.0};
        //  Light position
        float lightX = lightdistance*Cos(zh);
        float lightZ = lightdistance*Sin(zh);
        float Position[]  = {lightX,lightZ,ylight,1.0};
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
        
        nifFiles[i]->renderObjectTrishapes(shader[mode], sFlags);
        
        glPopMatrix();
        
    }
    //  Draw axes - no lighting from here on
    
    if (axes)
    {
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        glColor3f(1,0,0);
        glVertex3d(0.0,0.0,0.0);
        glVertex3d(len,0.0,0.0);
        glColor3f(0,1,0);
        glVertex3d(0.0,0.0,0.0);
        glVertex3d(0.0,len,0.0);
        glColor3f(0,0,1);
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

void nifQGLWidget::addShader(QString vert,QString frag,QString names){
	// Taken Directly from CUgl
   QOpenGLFunctions glf(QOpenGLContext::currentContext());
   QStringList name = names.split(',');
   QOpenGLShaderProgram* prog = new QOpenGLShaderProgram;
   //  Vertex shader
   if (vert.length() && !prog->addShaderFromSourceFile(QOpenGLShader::Vertex,vert))
      Fatal("Error compiling "+vert+"\n"+prog->log());
   //  Fragment shader
   if (frag.length() && !prog->addShaderFromSourceFile(QOpenGLShader::Fragment,frag))
      Fatal("Error compiling "+frag+"\n"+prog->log());
   //  Bind Attribute Locations
   for (int k=0;k<name.size();k++)
      if (name[k].length())
         glf.glBindAttribLocation(prog->programId(),k,name[k].toLatin1().data());
   //  Link
   if (!prog->link())
      Fatal("Error linking shader\n"+prog->log());
   //  Push onto stack
   else
      shader.push_back(prog);
}

void nifQGLWidget::setShader(int sel)
{
   if (sel>=0 && sel<shader.length())
      mode = sel;
   //  Request redisplay
   update();
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~ Mouse Events ~~~~~~~~~~~*/
/*~~~~~~~~~~-~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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
		dim += 10.0;
	}else{
		dim -= 10.0;
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
		filename = "cuirass_1.nif";
	}else if(spawncounter%3 == 0){
		filename = "glowingmushroom01.nif";
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
	if(filename != ""){
		nifFiles.push_back(new nifFileController(filename, 0)); //create a new nifFileController with the object's default translation.
		emit newObj(nifFiles.size()-1, filename);
		update();
	}
	
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

void nifQGLWidget::toggleShaderFlags(float flags[9]){
	for(int i = 0; i < 9; i++){
		sFlags[i] = flags[i];
	}
}

void nifQGLWidget::Fatal(QString message)
{
   QMessageBox::critical(this,"CUgl",message);
   QApplication::quit();
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
