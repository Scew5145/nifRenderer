//
//  nifRenderer.c
//  
//
//  Created by Scott Ewing on 10/16/17.
//
//


#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <time.h>
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

#define Cos(th) cos(3.1415926/180*(th))
#define Sin(th) sin(3.1415926/180*(th))

using namespace Niflib;
using namespace std;

unsigned int texture;



int light=1;      //  Lighting

int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
int axes=1;       //  Display axes
double asp=1;
double dim=100.0;
int fov=55;       //  Field of view (for perspective)


// Light values
int one       =   1;  // Unit value
int lightdistance  =   15;  // Light distance
int inc       =  10;  // Ball increment
int smooth    =   1;  // Smooth/Flat shading
int local     =   0;  // Local Viewer Model
int emission  =   0;  // Emission intensity (%)
int ambient   =  30;  // Ambient intensity (%)
int diffuse   = 100;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shiny   =   1;  // Shininess (value)
int zh        =  90;  // Light azimuth
float ylight  =   0;  // Elevation of light

int spawncounter = 0;//Controls which thing to spawn.

void Project(double fov,double asp,double dim)
{
    //  Tell OpenGL we want to manipulate the projection matrix
    glMatrixMode(GL_PROJECTION);
    //  Undo previous transformations
    glLoadIdentity();
    //  Perspective transformation
    if (fov)
        gluPerspective(fov,asp,dim/16,16*dim);
    //  Orthogonal transformation
    else
        glOrtho(-asp*dim,asp*dim,-dim,+dim,-dim,+dim);
    //  Switch to manipulating the model matrix
    glMatrixMode(GL_MODELVIEW);
    //  Undo previous transformations
    glLoadIdentity();
}

struct triTextureData{
    //This is a struct for mostly holding the texture IDs for each trishape.
    vector<string> textureFileName;
    vector<unsigned int> textureID;
};

/*
 * niFileController Class
 * A class for handling the loading and rendering of a nif.
 * For now, the file controller will force the user to load it using a filename. No blank file controllers.
 */
class nifFileController{
private:
    string fileName;
    NiNodeRef root;
    vector<triTextureData> triTextures;
    vector< vector<float> > triTrans;
    vector< vector<float> > triRot;
    //Most files will only use the 0,1,4, and 5 slots
    vector<unsigned int> textureIDs;
    void drawTriShape(NiTriShapeRef tShape, int triID){
        
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
        //cout << "starting forloop\n";
        for(int i = 0; i < tris.size(); i++){
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
            //cout << vertA << "\n";
        }
        glDisable(GL_TEXTURE_2D);
        //cout << "done \n";
        glPopMatrix();
        glEnd();
        

    }
public:
    nifFileController(string);
    void renderObjectTrishapes(){
        //cout << "We'll be rendering the following object: \n";
        //cout << root->asString();
        vector<NiAVObjectRef> children = root->GetChildren();
        int counter = 0;
        //If the child is a NiTriShape, pass it to the private function to parse and render the object.
        for(int i = 0; i < children.size(); i++){
            if(children[i]->GetType().IsSameType(NiTriShape::TYPE)){
                //cout << "Rendering " << i << "\n";
                drawTriShape(DynamicCast<NiTriShape>(children[i]), counter);
                counter++;
            }
        }
        //For now, trishapes will only be rendered from the first level of children.
        //Eventually, this may need to change. I've never encountered a .nif That contains NiTriShapes below the first level.
    }
};

nifFileController::nifFileController(string _filename){
    fileName = _filename;

    root = DynamicCast<NiNode>(ReadNifTree(fileName));
    //TODO: append all present textures to the object's textureList. For now, static for testing purposes.
    vector<NiAVObjectRef> children = root->GetChildren();
    
    
    for(int i = 0; i < children.size(); i++){
        if(children[i]->GetType().IsSameType(NiTriShape::TYPE)){
            //Temporary random location when created
            vector<float> randTrans;
            randTrans.push_back((rand()%100) - 50);
            randTrans.push_back((rand()%100) - 50);
            randTrans.push_back((rand()%100) - 50);
            vector<float> randRot;
            randRot.push_back(rand()%360-180);
            randRot.push_back((rand()%200)/200-100);
            randRot.push_back((rand()%200)/200-100);
            randRot.push_back((rand()%200)/200-100);
            triRot.push_back(randRot);
            triTrans.push_back(randTrans);
            //Grab the texture and append it to the texture list.
            triTextureData texDataIterator;
            
            //This looks messy but I'm just getting the shader property
            BSLightingShaderPropertyRef lsProperties = DynamicCast<BSLightingShaderProperty>(DynamicCast<NiTriShape>(children[i])->GetBSProperty(0));
            
            if(lsProperties != NULL){
                texDataIterator.textureFileName = lsProperties->GetTextureSet()->GetTextures();
                cout << texDataIterator.textureFileName.size() << endl;
                //Generate each TriShape's textures
                for(int j = 0; j < texDataIterator.textureFileName.size(); j++){
                    cout << texDataIterator.textureFileName[j] << endl;
                    if(strcmp(texDataIterator.textureFileName[j].c_str(), "") == 0){
                        texDataIterator.textureID.push_back(0);
                    }else{
                        texDataIterator.textureID.push_back(loadDDS(texDataIterator.textureFileName[j].c_str()));
                    }
                    cout << "texID for iterator:" << texDataIterator.textureID[j] << endl;
                    cout << "filename: " <<  texDataIterator.textureFileName[j] << endl;
                    
                }
            }else{
                cout << "No texture set. Ignoring." << endl;
            }
            triTextures.push_back(texDataIterator);
            cout << "size" << triTextures.size() << endl;
            
        }
    }
    //textureList.push_back(loadDDS("helmetplate_n.dds"));
}


vector<nifFileController> nifFiles;

static void Vertex(double th,double ph)
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
static void ball(double x,double y,double z,double r)
{
    int th,ph;
    float yellow[] = {1.0,1.0,1.0,1.0};
    float Emission[]  = {0.0,0.0,0.01*emission,1.0};
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



void idle()
{
    //  Elapsed time in seconds
    double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
    zh = fmod(90*t,360.0);
    //  Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}


void special(int key,int x,int y)
{
    
    //  Right arrow key - increase angle by 5 degrees
    if (key == GLUT_KEY_RIGHT){
        //cout << "doing stuff \n";
        th += 5;
    }
    //  Left arrow key - decrease angle by 5 degrees
    else if (key == GLUT_KEY_LEFT){
        //cout << "doing stuff2 \n";
        th -= 5;
    }
    //  Up arrow key - increase elevation by 5 degrees
    else if (key == GLUT_KEY_UP){
        //cout << "doing stuff3 \n";
        ph += 5;
    }
    //  Down arrow key - decrease elevation by 5 degrees
    else if (key == GLUT_KEY_DOWN){
        //cout << "doing stuff4 \n";
        ph -= 5;
    }
    //  Keep angles to +/-360 degrees
    th %= 360;
    ph %= 360;
    Project(0,asp,dim);
    //  Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

void reshape(int width,int height)
{
    //  Ratio of the width to the height of the window
    double w2h = (height>0) ? (double)width/height : 1;
    //  Set the viewport to the entire window
    glViewport(0,0, width,height);
    //  Tell OpenGL we want to manipulate the projection matrix
    glMatrixMode(GL_PROJECTION);
    //  Undo previous transformations
    glLoadIdentity();
    //  Orthogonal projection
    glOrtho(-w2h*dim,+w2h*dim, -dim,+dim, -dim,+dim);
    //  Switch to manipulating the model matrix
    glMatrixMode(GL_MODELVIEW);
    //  Undo previous transformations
    glLoadIdentity();
    glutPostRedisplay();
}

void key(unsigned char ch,int x,int y)
{
    //  Exit on ESC
    if (ch == 27)
        exit(0);
    //  Reset view angle
    else if (ch == '0')
        th = ph = 0;
    //  Toggle axes
    else if (ch == 'a' || ch == 'A')
        axes = 1-axes;
    //  Tell GLUT it is necessary to redisplay the scene
    else if (ch == 'r' || ch == 'r'){
        spawncounter++;
        if(spawncounter%3 == 1){
            nifFiles.push_back(nifFileController("Helmet.nif"));
        }else if(spawncounter%3 == 2){
            nifFiles.push_back(nifFileController("silverplate01.nif"));
        }else if(spawncounter%3 == 0){
            nifFiles.push_back(nifFileController("upperchair01.nif"));
        }
        
    }
    glutPostRedisplay();
}

/*
 * This function is called by GLUT to display the scene
 */
void display()
{
    const double len=2.0;  //  Length of axes
    //  Erase the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //  Enable Z-buffering in OpenGL
    glEnable(GL_DEPTH_TEST);
    //  Undo previous transformations
    glLoadIdentity();
    //  Perspective - set eye position
    glRotatef(ph,1,0,0);
    glRotatef(th,0,1,0);
    
    if (light)
    {
        //  Translate intensity to color vectors
        float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
        float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
        float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
        //  Light position
        float Position[]  = {lightdistance*Cos(zh),ylight,lightdistance*Sin(zh),1.0};
        //  Draw light position as ball (still no lighting here)
        glColor3f(1,1,1);
        ball(Position[0],Position[1],Position[2] , 0.1);
        
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
    for(int i = 0; i < nifFiles.size(); i++){
        glPushMatrix();
        
        nifFiles[i].renderObjectTrishapes();
        glPopMatrix();
        
    }
    //helmetFile.renderObjectTrishapes();
    //  Draw axes - no lighting from here on

    glColor3f(1,1,1);
    if (axes)
    {
        glBegin(GL_LINES);
        glVertex3d(0.0,0.0,0.0);
        glVertex3d(len,0.0,0.0);
        glVertex3d(0.0,0.0,0.0);
        glVertex3d(0.0,len,0.0);
        glVertex3d(0.0,0.0,0.0);
        glVertex3d(0.0,0.0,len);
        glEnd();
        //  Label axes
        /*glRasterPos3d(len,0.0,0.0);
        Print("X");
        glRasterPos3d(0.0,len,0.0);
        Print("Y");
        glRasterPos3d(0.0,0.0,len);
        Print("Z");*/
    }
    
    

    
    //  Render the scene and make it visible
    glFlush();
    glutSwapBuffers();
}
/*
 * GLUT based Hello World
 */
int main(int argc,char* argv[])
{
    //  Initialize GLUT
    
    srand (time(NULL));
    //  Request double buffered, true color window with Z buffering at 600x600
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    
    glutInit(&argc,argv);
    //  Create window
    glutCreateWindow("nifRenderer");
    //  Register function used to display scene
    glutInitWindowSize(600,600);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    //  Tell GLUT to call "reshape" when the window is resized
    glutReshapeFunc(reshape);
    
    //  Tell GLUT to call "special" when an arrow key is pressed
    glutSpecialFunc(special);
    glutKeyboardFunc(key);
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    //texture = loadDDS("helmetplate.dds");
    //  Pass control to GLUT for events
    glutMainLoop();
    //  Return to OS
    return 0;
}
