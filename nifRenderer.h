//nifRender.h

#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


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

using namespace Niflib;
using namespace std;

#ifndef NifRenderer_H
#define NifRenderer_H

struct triTextureData{
    //This is a struct for holding the texture IDs for each trishape with relation to their filename.
    //Index in each vector should be equivalent; ex: textureFileName[1] should be the filename for textureID[1] 
    vector<string> textureFileName;
    vector<unsigned int> textureID;
};

//Controls the rendering of a single nif object
//TODO: add set/get for translation, rotation, textures
class nifFileController{
private:
    string fileName;
    NiNodeRef root; //base node that's loaded when the object is created
    
    /*
     * The next three vectors hold information about the trishapes within the object.
     * triTextures holds each trishape's list of textures
     * triTrans holds their translation data
     * triRot holds their rotational data.
     * Importantly, they're held in vectors because nifs can hold more than one TriShape.
     */
    vector<triTextureData> triTextures;
    vector< vector<float> > triTrans; 
    vector< vector<float> > triRot;
        
    vector< vector<float> > maxCoords; //Maximum edge points for the trishape's bounding box (XYZ)
    vector< vector<float> > minCoords; //Minimum edge points for the trishape's bounding box (XYZ)
    vector< vector<float> > midPoint; // The center of the object. not weighted by vertex, literally just an avg between minCoords and maxCoords
    int rendering; //Toggle for describing whether or not the object is currently being rendered
    int renderingBox; //Toggle for describing whether or not the object's boundaries are being drawn
    
    
    void drawTriShape(NiTriShapeRef tShape, int triID);
    void drawBoundingBox(int triID);
public:
    nifFileController(string,int useRandom);
    ~nifFileController();
    void renderObjectTrishapes();
    void setRenderingBox(int val);
    void setTransXtShape(int tShapeIndex, float value);
    void setTransYtShape(int tShapeIndex, float value);
    void setTransZtShape(int tShapeIndex, float value);
    void setRotAngletShape(int tShapeIndex, float value);
    void setRotXtShape(int tShapeIndex, float value);
    void setRotYtShape(int tShapeIndex, float value);
    void setRotZtShape(int tShapeIndex, float value);
    void setTransRotTShape(string valueType, int tShapeIndex, float value); //A generic setter for all Translation or Rotation values.
    string getInfoString();
};

#endif /* NifRenderer_H */
