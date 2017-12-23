# nifRenderer
Scott Ewing

To Compile and Run:
Use the makefile attached to compile the source code by running the make command. Using ./nifRenderer will start the program.
Controls:
R: generate a new nif object in the scene in a random location. 
The arrow keys will rotate the camera around the scene. 

For the sake of space, I only included a few .nif files and textures, but the idea is that you can use the tool to load just about any nif and mess with it! So far, it's worked with the 12 files I've tried it with.
Some objects have multiple parts (triShapes) such as the helmet. Right now, these extra parts are rendered in a different rotation and translation for each separate piece. when this is finished, There'll be an option to move them either together or alone.

What's finished out of my goals:
- The ability to read .nif files for object information
- The ability to read a .dds texture and apply it to an object
- A class for manipulating .nif objects and making them appear on the screen
- Properly applied normals to the 3d data of the object

What's left to do:
All I have to do now is build a better UI for the project.
When I manage to finish that, I'll be working on getting the normal maps working (technically a strech goal).

Known Issues:
- Building spits out a bunch (5 of them, rather) of  warnings associated with the niflib library. I haven't spent the time needed to fix them, but I will if I have time. 
