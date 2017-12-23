# nifRenderer
----
Scott Ewing

To Compile and Run:

Run the following commands in the base folder:
* qmake
* make
* Next, run the program nifRenderer (./nifRenderer or some equivalent)


#Controls:

Use the scroll wheel to move in and out with the camera.

Click and drag the render window to rotate the camera (around itself)

Use "Load file" button to pick any .nif file.

Use "Create Random Object" to create an object in a random location (centered near the origin)

Use the dropdown menu to select a .nif object in the scene.

#What it Does:

- The full program is an interface to load, orient, and display multiple .nif objects. 
- The program will automatically load the .dds texture files that are mentioned in the .nif files and attach them to the correct sub-object (triShape)
- The program automatically calculates a Bounding Box for displaying the currently selected object.
- Supports as many objects in the scene as your computer can handle.


#Other Less Important info:

For the sake of space, I only included a few .nif files and textures, but the idea is that you can use the tool to load just about any nif and mess with it! So far, it's worked with the 14 files I've tried it with.

Some objects have multiple parts (triShapes) such as the helmet. Right now, these extra parts are rendered in a different rotation and translation for each separate piece. when this is finished, There'll be an option to move them either together or alone.

#Files included for loading:

- helmet_1.nif
- upperchair01.nif
- silverplate01.nif
- JuggernautCuiriass_1.nif

The reasons the files provided were chosen was because they represent different 'classes' of nif files. The Chair is a static object that won't move under any conditions in the game world, the plate is a 'clutter' object meant to sit in the overworld and interacted with, and the helmet is an armor piece meant to be worn.

The final file, the JuggernautCuirass, I made a few years ago. I didn't include the textures, but the idea is that it can be loaded without them.

I wanted to show that I managed to get this program to parse a wide variety of situations that .nif files can describe.

