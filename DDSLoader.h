#include <sstream>
#include <iostream>
#include <fstream>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#ifndef DDSLoader
#define DDSLoader

GLuint loadDDS(const char * imagepath);

#endif /* DDSLoader_h */
