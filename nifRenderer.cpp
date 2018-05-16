//
//  nifRenderer.c
//  
//
//  Created by Scott Ewing on 10/16/17.
//	
//	This file contains construction functions for any objects within the scene.
//	The nifFileController class is for rendering a single .nif file.
//


#include <sstream>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <math.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

//QtOpenGL
#include <QtOpenGL>

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
				//Vector3 transInput = children[i]->GetLocalTranslation();
				
				rotTemp.push_back(0);
				rotTemp.push_back(0);
				rotTemp.push_back(0);
				rotTemp.push_back(0);
				
				transTemp.push_back(0);
				transTemp.push_back(0);
				transTemp.push_back(0);
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
            shaderProperties sPropTemp;
            //This looks messy but I'm just getting the shader property
            BSLightingShaderPropertyRef lsProperties = DynamicCast<BSLightingShaderProperty>(DynamicCast<NiTriShape>(children[i])->GetBSProperty(0));
            if(lsProperties != NULL){
				// Get the shader properties
				
				sPropTemp.emissiveColor = lsProperties->GetEmissiveColor();
				sPropTemp.emissiveMultiple = lsProperties->GetEmissiveMultiple();
				sPropTemp.specularColor = lsProperties->GetSpecularColor();
				sPropTemp.specularStrength = lsProperties->GetSpecularStrength();
				sPropTemp.glossiness = lsProperties->GetGlossiness();
                
                texDataIterator.textureFileName = lsProperties->GetTextureSet()->GetTextures();
                //texDataIterator.textureFileName[4] = "Chitin_e_ebony.dds";
                //Generate each TriShape's textures
                // TODO: if the object already exists, don't make a second texture for an object that already exists.
                // Static texture stuff? Use a flyweight type of thing?
                // For now this works, though.
                for(unsigned int j = 0; j < texDataIterator.textureFileName.size(); j++){
                    if(strcmp(texDataIterator.textureFileName[j].c_str(), "") == 0){
                        texDataIterator.textureID.push_back(0);
                    }else{
						texDataIterator.textureID.push_back(loadDDS(texDataIterator.textureFileName[j].c_str()));
						//if(j = )
                    }
                    
                }
            }else{
				sPropTemp.emissiveColor.r = 0.0; sPropTemp.emissiveColor.g = 0.0; sPropTemp.emissiveColor.b = 0.0;
				sPropTemp.emissiveMultiple = 0.0;
				sPropTemp.specularColor.r = 1.0; sPropTemp.specularColor.g = 1.0; sPropTemp.specularColor.b = 1.0;
				sPropTemp.specularStrength = 1.0;
				sPropTemp.glossiness = 32.0;
			}
			sProperties.push_back(sPropTemp);
            // TODO: this might have scoping issues. worth looking at eventually
            triTextures.push_back(texDataIterator);
            
            
        }
    }
    // Initialize the triShapes
    initTris();
    // TODO: It might be worth removing the root of the NiNode after initialization is done now that all of the data is stored in float arrays.
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
void nifFileController::initTris(){
	vector<NiAVObjectRef> children = root->GetChildren();
	int counter = 0;
	for(unsigned int i = 0; i < children.size(); i++){
		if(children[i]->GetType().IsSameType(NiTriShape::TYPE)){
			NiTriShapeRef tShape = DynamicCast<NiTriShape>(children[i]);
			NiTriShapeDataRef tData = DynamicCast<NiTriShapeData>(tShape->GetData());
			int numTriangles = tData->GetTriangles().size();
			triCount.push_back(numTriangles);
			
			// check to see if the normals are already stored in the object.
			// If they aren't, just store a NULL entry in each of the N/B/T arrays.
			if(tData->GetNormals().size()){ 
				Norms.push_back(new float[3*3*numTriangles]);
				Tans.push_back(new float[3*3*numTriangles]);
				Bitans.push_back(new float[3*3*numTriangles]);
			}else{
				Norms.push_back(NULL);
				Tans.push_back(NULL);
				Bitans.push_back(NULL);
			}
			Verts.push_back(new float[3*3*numTriangles]);
			
			TexCoords.push_back(new float[3*2*numTriangles]);
			if(tData->GetColors().size()){
				Colors.push_back(new float[3*4*numTriangles]);
			}else{
				
				Colors.push_back(new float(-1));
			}
			initTriShape(tShape, counter);
			counter++;
		}
	}
	
}
void nifFileController::initTriShape(NiTriShapeRef tShape, int triID){
	NiTriShapeDataRef tData = DynamicCast<NiTriShapeData>(tShape->GetData());
	
	// Choose the correct float* array to output to:
	float* oVerts = Verts[triID];
	float* oNorms = Norms[triID];
	float* oColors = Colors[triID];
	float* oTans = Tans[triID];
	float* oBitans = Bitans[triID];
	float* oTCoords = TexCoords[triID];
	// Grab the data from tData that we'll be putting in the float arrays
	vector<Triangle> tris = tData->GetTriangles();
	vector<Vector3> verts = tData->GetVertices();
	vector<Vector3> norms = tData->GetNormals();
	vector<Vector3> tans = tData->GetTangents();
	vector<Vector3> bitans = tData->GetBitangents();
	vector<Color4> vertColors = tData->GetColors();
	vector<TexCoord> texCoords = tData->GetUVSet(0);
	if(vertColors.size() != 0){
		for(unsigned int i = 0; i < tris.size(); i++){
			Triangle triIterator = tris[i];
			for(unsigned int j = 0; j < 3; j++){
				
				*oColors++ = vertColors[triIterator[j]].r;
				*oColors++ = vertColors[triIterator[j]].g;
				*oColors++ = vertColors[triIterator[j]].b;
				*oColors++ = vertColors[triIterator[j]].a;
			}
		}
	}
	for(unsigned int i = 0; i < tris.size(); i++){
		Triangle triIterator = tris[i];
		for(unsigned int j = 0; j < 3; j++){
			
			*oVerts++ = verts[triIterator[j]][0];
			*oVerts++ = verts[triIterator[j]][1];
			*oVerts++ = verts[triIterator[j]][2];
			if(norms.size()){ // if the Normals aren't in tangent space, we won't have per-vertex normals/tans/bitans stored in the object.
				*oNorms++ = norms[triIterator[j]][0];
				*oNorms++ = norms[triIterator[j]][1];
				*oNorms++ = norms[triIterator[j]][2];
			}else{ // if this is the case, our normals are instead in the tex_n texture ONLY. That means shader shit has to happen. 
			
			}
			if(tans.size()){
				*oTans++ = tans[triIterator[j]][0];
				*oTans++ = tans[triIterator[j]][1];
				*oTans++ = tans[triIterator[j]][2];
			}
			if(bitans.size()){
				*oBitans++ = bitans[triIterator[j]][0];
				*oBitans++ = bitans[triIterator[j]][1];
				*oBitans++ = bitans[triIterator[j]][2];
			}
			
			*oTCoords++ = texCoords[triIterator[j]].u;
			*oTCoords++ = texCoords[triIterator[j]].v;
		}
	}
	
}
void nifFileController::drawTriShape(int triID, QOpenGLShaderProgram* shader, float flags[9]){
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
	
	QOpenGLFunctions glf(QOpenGLContext::currentContext());
	
	glPushMatrix();
	glTranslated(triTrans[triID][0],triTrans[triID][1],triTrans[triID][2]);
	glRotated(triRot[triID][0],triRot[triID][1],triRot[triID][2],triRot[triID][3]);
	
	glEnable(GL_LIGHTING);
	
	//Set up the texture stuff if the trishape has a texture
	if(triTextures[triID].textureFileName.size() != 0){
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glEnable(GL_TEXTURE_2D);
	}
	
	//Set the shader flags, find the proper textures
	GLfloat shaderTexFlags[9];
	for(unsigned int texIter = 0; texIter < triTextures[triID].textureID.size(); texIter++){
		if(triTextures[triID].textureID[texIter] != 0){
			shaderTexFlags[texIter] = 1*flags[texIter];
			//If there's a texture, bind it to the appropriate slot
			glActiveTexture(GL_TEXTURE0+texIter);
			glBindTexture(GL_TEXTURE_2D,triTextures[triID].textureID[texIter]);
		}else{
			shaderTexFlags[texIter] = 0;
		}
	}
	if(Norms[triID] == NULL){
		// We have no stored normals in our model. let the texture handle it by setting shader flag 8 to 1
		shaderTexFlags[8] = 1*flags[8];
	}else{
		shaderTexFlags[8] = 0;
	}
	
	//Texture values
	shader->setUniformValue("tex_d" ,0); // Diffuse texture
	shader->setUniformValue("tex_n" ,1); // Normal map texture
	shader->setUniformValue("tex_g" ,2); // Glow map texture
	// TODO: greyscale height map in slot 3
	shader->setUniformValue("tex_e",4); // The cubemap
	shader->setUniformValue("tex_m",5); // Enviroment/cube mask texture. Controls specular intensity (also controlled by Normal map alpha value if this doesn't exist)
	shader->setUniformValue("tex_s",6);
	shader->setUniformValueArray("texFlags",shaderTexFlags, 9,1);
	
	// Other render properties
	QColor eColor; 
	eColor.setRgbF(sProperties[triID].emissiveColor.r, sProperties[triID].emissiveColor.g, sProperties[triID].emissiveColor.b);
	QColor sColor;
	sColor.setRgbF(sProperties[triID].specularColor.r, sProperties[triID].specularColor.g, sProperties[triID].specularColor.b);
	shader->setUniformValue("emissiveColor",eColor);
	shader->setUniformValue("emissiveMultiple",sProperties[triID].emissiveMultiple);
	shader->setUniformValue("specularColor",sColor);
	shader->setUniformValue("specularStrength", sProperties[triID].specularStrength);
	shader->setUniformValue("glossiness", sProperties[triID].glossiness);
	
	// Point information arrays to the right places
	glf.glVertexAttribPointer(tanArrayLoc,3,GL_FLOAT,GL_FALSE,0,Tans[triID]);
	glf.glVertexAttribPointer(bitanArrayLoc,3,GL_FLOAT,GL_FALSE,0,Bitans[triID]);
	glVertexPointer(3,GL_FLOAT,0,Verts[triID]);
	glTexCoordPointer(2,GL_FLOAT,0,TexCoords[triID]);
	glNormalPointer(GL_FLOAT,0,Norms[triID]);
	
	//Enable arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glf.glEnableVertexAttribArray(tanArrayLoc);
	glf.glEnableVertexAttribArray(bitanArrayLoc);
	//Point the arrays to the correct locations
	if(Colors[triID][0] != -1){
		glColorPointer(4,GL_FLOAT,0,Colors[triID]);
	}else{
		glDisableClientState(GL_COLOR_ARRAY);
		glColor3f(1,1,1);
	}
	if(Norms[triID] == NULL){
		
		glDisableClientState(GL_NORMAL_ARRAY);
		glNormal3f(0,0,1);
		glf.glDisableVertexAttribArray(tanArrayLoc);
		glf.glDisableVertexAttribArray(bitanArrayLoc);
		
	}
	glDrawArrays(GL_TRIANGLES,0,triCount[triID]*3);
	
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glf.glDisableVertexAttribArray(tanArrayLoc);
	glf.glDisableVertexAttribArray(bitanArrayLoc);
	glPopMatrix();
	glEnd();
	
}

void nifFileController::drawBoundingBox(int triID){
	if(!renderingBox) return;
	
	//Grab the maximum coordinates and minimum coordinates for the associate TriShape
	vector<float> maxes = maxCoords[triID];
	vector<float> mins = minCoords[triID];
	glPushMatrix();
	glTranslated(triTrans[triID][0],triTrans[triID][1],triTrans[triID][2]);
	glRotated(triRot[triID][0],triRot[triID][1],triRot[triID][2],triRot[triID][3]);
	
	//Draw the box. I'll be drawing them as lines, because you still want to be able to see the actual object!
	glDisable(GL_LIGHTING);
	//Disable the textures of the object so that we get white lines instead of... blue, or grey, or red, or piss yellow lines
	for(unsigned int i = 0; i < 9; i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
	}
	glActiveTexture(GL_TEXTURE0);
	
	
	glBegin(GL_LINES);
	//Top 
	glColor4f(1.f,1.f,1.f,1.f);
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
	glPopMatrix();
}

void nifFileController::renderObjectTrishapes(QOpenGLShaderProgram* shader, float flags[9]){
	vector<NiAVObjectRef> children = root->GetChildren();
	int counter = 0;
	//If the child is a NiTriShape, pass it to the private function to parse and render the object.
	for(unsigned int i = 0; i < children.size(); i++){
		if(children[i]->GetType().IsSameType(NiTriShape::TYPE)){
			glPushMatrix();
			shader->bind();
			drawTriShape(counter, shader, flags);
			shader->release();
			
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
