# nifRenderer Project File
# Scott Ewing
#

# Header Files
HEADERS = nifRenderer.h DDSLoader.h nifQGLWidget.h viewer.h
HEADERS += niflib/include/niflib.h
HEADERS += niflib/include/obj/NiNode.h
HEADERS += niflib/include/obj/NiSkinInstance.h
HEADERS += niflib/include/obj/NiTriShape.h
HEADERS += niflib/include/obj/NiTriShapeData.h
HEADERS += niflib/include/obj/BSLightingShaderProperty.h
HEADERS += niflib/include/obj/BSShaderTextureSet.h

# Source Files
SOURCES = main.cpp nifRenderer.cpp DDSLoader.cpp nifQGLWidget.cpp viewer.cpp 

#Libraries
LIBS += -Lniflib/build/ -lniflib_static
LIBS += -lglut -lGLU

# Have to disable a pragma warning that comes from nifLib - I'm not even using the file that the warning comes from, but without doing this it'll spam warnings at you
QMAKE_CXXFLAGS += -Wno-unknown-pragmas

CONFIG += debug
QT += opengl
QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
