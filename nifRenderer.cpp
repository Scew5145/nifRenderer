//
//  nifRenderer.c
//  
//
//  Created by Scott Ewing on 10/16/17.
//	
//	This file contains construction functions for any objects within the scene.
//	The nifFileController class is for rendering a single .nif file.
//


// TODO
// Build the rest of the viewer
// Debug aggressively
// Pray it compiles on his computer

#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <math.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

//nifRenderer
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







nifFileController::nifFileController(string _filename, int useRandom = 1){
    fileName = _filename;
    root = DynamicCast<NiNode>(ReadNifTree(fileName));
	
	//Set up default rendering toggles to 'true'
	rendering = 1;
	renderingBox = 0;
	
    vector<NiAVObjectRef> children = root->GetChildren();
    
    
    for(unsigned int i = 0; i < children.size(); i++){
        if(children[i]->GetType().IsSameType(NiTriShape::TYPE)){
			vector<float> rotTemp;
			vector<float> transTemp;
			//random location when created unless otherwise specified
			if(useRandom){
				transTemp.push_back((rand()%100) - 50);
				transTemp.push_back((rand()%100) - 50);
				transTemp.push_back((rand()%100) - 50);
				rotTemp.push_back(rand()%360-180);
				rotTemp.push_back((float)((rand()%100))/100-0.5 );
				rotTemp.push_back((float)((rand()%100))/100-0.5 );
				rotTemp.push_back((float)((rand()%100))/100-0.5 );
				triRot.push_back(rotTemp);
				triTrans.push_back(transTemp);
			}else{
				Vector3 transInput = children[i]->GetLocalTranslation();
				
				rotTemp.push_back(0);
				rotTemp.push_back(0);
				rotTemp.push_back(0);
				rotTemp.push_back(0);
				
				transTemp.push_back(transInput[0]);
				transTemp.push_back(transInput[1]);
				transTemp.push_back(transInput[2]);
				triTrans.push_back(transTemp);
				triRot.push_back(rotTemp);
			}
			
			
			//Fill out the bounding box information
            NiTriShapeDataRef tData = DynamicCast<NiTriShapeData>(DynamicCast<NiTriShape>(children[i])->GetData());
            vector<Vector3> verts = tData->GetVertices();
            /*
             * Basic steps for this portion:
             * Iterate through the verticies, if it's the lowest/highest X, Y or Z coordinate I've seen so far, update minCoords or maxCoords.
             */
            vector<float> mins(3,0);
            vector<float> maxes(3,0);
            for(unsigned int vertIterator = 0; vertIterator < verts.size(); vertIterator++){
				Vector3 currentVert = verts[vertIterator];
				if(mins[0] > currentVert[0]){
					mins[0] = currentVert[0];
				}
				if(mins[1] > currentVert[1]){
					mins[1] = currentVert[1];
				}
				if(mins[2] > currentVert[2]){
					mins[2] = currentVert[2];
				}
				
				if(maxes[0] < currentVert[0]){
					maxes[0] = currentVert[0];
				}
				if(maxes[1] < currentVert[1]){
					maxes[1] = currentVert[1];
				}
				if(maxes[2] < currentVert[2]){
					maxes[2] = currentVert[2];
				}
			}
			//Now that we have our mins and maxes for the trishape, add it to our minCoords/maxCoords index.
			vector<float> centerPoint(3,0);
			centerPoint[0] = (mins[0]+maxes[0])/2;
			centerPoint[1] = (mins[1]+maxes[1])/2;
			centerPoint[2] = (mins[2]+maxes[2])/2;
			midPoint.push_back(centerPoint);
			maxCoords.push_back(maxes);
			minCoords.push_back(mins); 
            
            //Grab the texture and append it to the texture list.
            triTextureData texDataIterator;
            
            //This looks messy but I'm just getting the shader property
            BSLightingShaderPropertyRef lsProperties = DynamicCast<BSLightingShaderProperty>(DynamicCast<NiTriShape>(children[i])->GetBSProperty(0));
            
            if(lsProperties != NULL){
                texDataIterator.textureFileName = lsProperties->GetTextureSet()->GetTextures();
                //Generate each TriShape's textures
                for(unsigned int j = 0; j < texDataIterator.textureFileName.size(); j++){
                    if(strcmp(texDataIterator.textureFileName[j].c_str(), "") == 0){
                        texDataIterator.textureID.push_back(0);
                    }else{
                        texDataIterator.textureID.push_back(loadDDS(texDataIterator.textureFileName[j].c_str()));
                    }
                    
                }
            }else{
            }
            triTextures.push_back(texDataIterator);
            
        }
    }
}

nifFileController::~nifFileController() {
	// when the controller is deleted, free the associated textures.
	// glDeleteTexture will silently ignore textures with ID 0, so we can just iterate over the entire texture list.
	for(unsigned int i = 0; i < triTextures.size(); i++){
		for(unsigned int j = 0; j < triTextures[i].textureID.size(); j++){
			glDeleteTextures(1, &triTextures[i].textureID[j]);
		}
	}
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~ Draw Functions ~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void nifFileController::drawTriShape(NiTriShapeRef tShape, int triID){
	/* 
	 * Attributes of a NiTriShape and it's Children:
	 *  NiTriShapes are the base container for any 3d object data in skyrim. 
	 *  Thus, in order for us to render the object, we only need the information from these nodes.
	 *  Primarily, they serve as a root node for the following children:
	 * 1. NiTriShapeData, 
	 *      the place where the list of Vertex Coordinates are stored, as well as triangles and normals
	 *          Triangles are stored as 3 integers that represent indexes in the 'verticies' array.
	 *          Normals are stored as Vector3s, and their index lines up with the index of the verticies
	 * 2. BSLightingShaderProperty, 
	 *      the place where stuff like specular color, texture map mode, etc. are stored
	 *      Access as GetBSProperty(0)
	 * 2a.NiAlphaProperty
	 *      this node is for storing alpha values of stuff I don't really care about for this project (yet).
	 *      Accessed as GetBSProperty(1)
	 * There's also a few other children of the node that don't matter for this project - most related to animation or ingame collision.
	 * Within the NiTriShape node itself, Translation, Rotation, and Scale are notated.
	 */
	if(!rendering) return; //if the object is disabled, don't render it.
	 
	NiTriShapeDataRef tData = DynamicCast<NiTriShapeData>(tShape->GetData());
	
	BSLightingShaderPropertyRef lsProperties = DynamicCast<BSLightingShaderProperty>(tShape->GetBSProperty(0));
	
	//Get the texture info
	
	
	//Get the 3d data
	vector<Triangle> tris = tData->GetTriangles();
	vector<Vector3> verts = tData->GetVertices();
	vector<Vector3> norms = tData->GetNormals();
	vector<TexCoord> texCoords = tData->GetUVSet(0);
	glPushMatrix();
	glTranslated(triTrans[triID][0],triTrans[triID][1],triTrans[triID][2]);
	glRotated(triRot[triID][0],triRot[triID][1],triRot[triID][2],triRot[triID][3]);
	
	glEnable(GL_LIGHTING);
	
	//Set up the texture stuff if the trishape has a texture
	if(triTextures[triID].textureFileName.size() != 0){
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glEnable(GL_TEXTURE_2D);
		//Bind the diffuse texture (which will always be the first textureID)
		glBindTexture(GL_TEXTURE_2D,triTextures[triID].textureID[0]);
	}
	
	glShadeModel(GL_SMOOTH);
	
	//Set up openGL to draw tris
	glBegin(GL_TRIANGLES);
	glColor3f(1,1,1);
	for(unsigned int i = 0; i < tris.size(); i++){
		Triangle triIterator = tris[i];
		Vector3 vertA = verts[triIterator[0]];
		Vector3 vertB = verts[triIterator[1]];
		Vector3 vertC = verts[triIterator[2]];
		Vector3 normA = norms[triIterator[0]];
		Vector3 normB = norms[triIterator[1]];
		Vector3 normC = norms[triIterator[2]];
		
		glTexCoord2d(texCoords[triIterator[0]].u, texCoords[triIterator[0]].v);
		glNormal3d(normA[0], normA[1], normA[2]);
		glVertex3d(vertA[0],vertA[1],vertA[2]);
		
		glTexCoord2d(texCoords[triIterator[1]].u, texCoords[triIterator[1]].v);
		glNormal3d(normB[0], normB[1], normB[2]);
		glVertex3d(vertB[0],vertB[1],vertB[2]);
		
		glTexCoord2d(texCoords[triIterator[2]].u, texCoords[triIterator[2]].v);
		glNormal3d(normC[0],normC[1], normC[1]);
		glVertex3d(vertC[0],vertC[1],vertC[2]);
	}
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glEnd();
	
}

void nifFileController::drawBoundingBox(int triID){
	if(!renderingBox) return;
	
	//Grab the maximum coordinates and minimum coordinates for the associate TriShape
	vector<float> maxes = maxCoords[triID];
	vector<float> mins = minCoords[triID];
	
	//Draw the box. I'll be drawing them as lines, because you still want to be able to see the actual object!
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glColor3f(1,1,1);
	
	glBegin(GL_LINES);
	//Top 
	glVertex3d(maxes[0],maxes[1],maxes[2]); glVertex3d(mins[0],maxes[1],maxes[2]);
	glVertex3d(maxes[0],maxes[1],maxes[2]); glVertex3d(maxes[0],maxes[1],mins[2]);
	glVertex3d(mins[0],maxes[1],mins[2]); glVertex3d(mins[0],maxes[1],maxes[2]);
	glVertex3d(mins[0],maxes[1],mins[2]); glVertex3d(maxes[0],maxes[1],mins[2]);
	//Bottom
	glVertex3d(maxes[0],mins[1],maxes[2]); glVertex3d(mins[0],mins[1],maxes[2]);
	glVertex3d(maxes[0],mins[1],maxes[2]); glVertex3d(maxes[0],mins[1],mins[2]);
	glVertex3d(mins[0],mins[1],mins[2]); glVertex3d(mins[0],mins[1],maxes[2]);
	glVertex3d(mins[0],mins[1],mins[2]); glVertex3d(maxes[0],mins[1],mins[2]);
	//Mid Connectors
	glVertex3d(maxes[0],maxes[1],maxes[2]); glVertex3d(maxes[0],mins[1],maxes[2]);
	glVertex3d(maxes[0],maxes[1],mins[2]); glVertex3d(maxes[0],mins[1],mins[2]);
	glVertex3d(mins[0],maxes[1],maxes[2]); glVertex3d(mins[0],mins[1],maxes[2]);
	glVertex3d(mins[0],maxes[1],mins[2]); glVertex3d(mins[0],mins[1],mins[2]);
	glEnd();
	//glPopMatrix();
}

void nifFileController::renderObjectTrishapes(){
	vector<NiAVObjectRef> children = root->GetChildren();
	int counter = 0;
	//If the child is a NiTriShape, pass it to the private function to parse and render the object.
	for(unsigned int i = 0; i < children.size(); i++){
		if(children[i]->GetType().IsSameType(NiTriShape::TYPE)){
			glPushMatrix();
			drawTriShape(DynamicCast<NiTriShape>(children[i]), counter);
			drawBoundingBox(counter);
			glPopMatrix();
			counter++;
		}
	}
	//For now, trishapes will only be rendered from the first level of children.
	//This may need to change, but I've never encountered a .nif That contains NiTriShapes below the first level.
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~ Get/Set Functions ~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

//Translation
void nifFileController::setTransXtShape(int tShapeIndex, float value){
	triTrans[tShapeIndex][0] = value;
}
void nifFileController::setTransYtShape(int tShapeIndex, float value){
	triTrans[tShapeIndex][1] = value;
}
void nifFileController::setTransZtShape(int tShapeIndex, float value){
	triTrans[tShapeIndex][2] = value;
}

//Rotation
void nifFileController::setRotAngletShape(int tShapeIndex, float value){
	triRot[tShapeIndex][0] = value;
}
void nifFileController::setRotXtShape(int tShapeIndex, float value){
	triRot[tShapeIndex][1] = value;
}
void nifFileController::setRotYtShape(int tShapeIndex, float value){
	triRot[tShapeIndex][2] = value;
}
void nifFileController::setRotZtShape(int tShapeIndex, float value){
	triRot[tShapeIndex][3] = value;
}

void nifFileController::setRenderingBox(int val){
	renderingBox = val;
}


string nifFileController::getInfoString(){
	//All this function does is grab the important information about the object and spit it out in an easily parsible string.
	
	ostringstream oss;
	
	//First Line is basic info: Filename, Number of triShapes, rendering flag, renderingBox flag 
	oss << fileName << "," << triTextures.size() << "," << rendering << "," << renderingBox << '\n';	
	//Second Line is texture info. For now, just a list of the diffuse texture's filenames.
	for(unsigned int i = 0; i < triTextures.size(); i++){
		if(triTextures[i].textureID.size()){
			oss << triTextures[i].textureFileName[0];
		}else{
			oss << "[NO TEXTURE ATTACHED]";
		}
		oss << ",";
	}
	oss << '\n';
	//Third Line is each trishapes's translation and rotation information. 
	//Semicolons separate trishapes. Order of info is tX, tY, tZ, rA, rX, rY, rZ;
	for(unsigned int i = 0; i < triTrans.size(); i++){
		oss << triTrans[i][0] << "," << triTrans[i][1] << "," << triTrans[i][2] << "," << triRot[i][0] << "," << triRot[i][1] << "," << triRot[i][2] << "," << triRot[i][3] << ";";
	} 
	oss << '\n';
	//Final line is bounding box information. Semicolons again separate trishapes, order of info is minX,minY,minZ,maxX,maxY,maxZ;
	for(unsigned int i = 0; i < maxCoords.size(); i++){
		oss << minCoords[i][0] << "," << minCoords[i][1] << "," << minCoords[i][2] << "," << maxCoords[i][0] << "," << maxCoords[i][1] << "," << maxCoords[i][2] << ";";
	}
	oss << '\n';
	
	return oss.str();
}
