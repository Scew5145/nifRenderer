//
//  nifRenderer.c
//  
//
//  Created by Scott Ewing on 10/16/17.
//
//


#include <sstream>
#include <iostream>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "niflib/include/niflib.h"
#include "niflib/include/obj/NiNode.h"
#include "niflib/include/obj/NiSkinInstance.h"
#include "niflib/include/obj/NiTriShape.h"
#include "niflib/include/obj/NiTriShapeData.h"
#include "niflib/include/obj/BSLightingShaderProperty.h"

using namespace Niflib;
using namespace std;


/*
 * niFileController Class
 * A class for handling the loading and rendering of a nif.
 * For now, the file controller will force the user to load it using a filename. No blank file controllers.
 */
class nifFileController{
private:
    string fileName;
    NiNodeRef root;
    void drawTriShape(NiTriShapeRef tShape){
        cout << "Rendering SubObject:\n";
        cout << tShape->asString();
        
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
         * 2a.NiAlphaProperty
         *      this node is for storing alpha values of stuff I don't really care about for this project.
         * There's also a few other children of the node that don't matter for this project - most related to animation or ingame collision.
         * Within the NiTriShape node itself, Translation, Rotation, and Scale are notated.
         */
        NiTriShapeDataRef tData = DynamicCast<NiTriShapeData>(tShape->GetData());
        BSLightingShaderPropertyRef lsProperties = DynamicCast<BSLightingShaderProperty>(tShape->GetBSProperty(0));
        
        //TODO: Finish this
        //Start with setting up lighting properties for the TriShape using lsProperties
        //Next, sequentually render the object by iterating through the Triangles vector.
        //Ex:
        // Normal[Tri[0]] Vert[Tri[0]]
        // Normal[Tri[1]] Vert[Tri[1]]
        // Normal[Tri[2]] Vert[Tri[2]]
        // When this is finished, look in to applying textures

    }
public:
    nifFileController(string);
    void renderObjectTrishapes(){
        //cout << "We'll be rendering the following object: \n";
        //cout << root->asString();
        vector<NiAVObjectRef> children = root->GetChildren();
        //If the child is a NiTriShape, pass it to the private function to parse and render the object.
        for(int i = 0; i < children.size(); i++){
            if(children[i]->GetType().IsSameType(NiTriShape::TYPE)){
                //cout << "Rendering " << i << "\n";
                drawTriShape(DynamicCast<NiTriShape>(children[i]));
            }
        }
        //For now, trishapes will only be rendered from the first level of children.
        //Eventually, this may need to change. I've never encountered a .nif That contains NiTriShapes below the first level.
    }
};

nifFileController::nifFileController(string _filename){
    fileName = _filename;
    root = DynamicCast<NiNode>(ReadNifTree(fileName));
}



/*
 * This function is called by GLUT to display the scene
 */
void display()
{
    //  Clear screen
    glClear(GL_COLOR_BUFFER_BIT);
    //  Draw triangle
    glBegin(GL_POLYGON);
    glVertex2f( 0.0, 0.5);
    glVertex2f( 0.5,-0.5);
    glVertex2f(-0.5,-0.5);
    glEnd();
    //  Make scene visible
    glFlush();
}
/*
 * GLUT based Hello World
 */
int main(int argc,char* argv[])
{
    //  Initialize GLUT
    //std::cout <<  "words\n";
    
    nifFileController helmetFile = nifFileController("helmet.nif");
    helmetFile.renderObjectTrishapes();
    glutInit(&argc,argv);
    //  Create window
    glutCreateWindow("nifRenderer");
    //  Register function used to display scene
    glutDisplayFunc(display);
    //  Pass control to GLUT for events
    glutMainLoop();
    //  Return to OS
    return 0;
}
