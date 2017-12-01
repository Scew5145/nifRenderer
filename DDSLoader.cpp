#include <sstream>
#include <stdio.h>
#include <iostream>
#include <fstream>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


GLuint loadDDS(const char * imagepath){
    
    unsigned char header[124];
    FILE *fp;
    //std::cout << "TID :" <<  strrchr(imagepath,'\\')+1 << std::endl;
    
    fp = fopen(imagepath, "rb");
    if (fp == NULL){
        //const char * justimagename = strrchr(imagepath,'/');
        fp = fopen(strrchr(imagepath,'\\')+1, "rb");
        if(fp == NULL){//If the base file name doesn't work either, just return
            return 0;
        }
        
    }
    
    char filecode[4];
    fread(filecode, 1, 4, fp);
    if (strncmp(filecode, "DDS ", 4) != 0) {
        fclose(fp);
        return 0;
    }
    
    //Have to start by reading the header.
    //Most importantly, we need to know our dimensions, the size of the image, if mipmaps are present, and which format of .dds file we're using.
    //Skyrim can use any format for .dds files, but mostly it should use DXT5 due to specular maps using the alpha channel for the normal map.
    fread(&header, 124, 1, fp);
    
    unsigned int height      = *(unsigned int*)&(header[8 ]);
    unsigned int width         = *(unsigned int*)&(header[12]);
    unsigned int linearSize     = *(unsigned int*)&(header[16]);
    unsigned int mipMapCount = *(unsigned int*)&(header[24]);
    char fourCC[5]; //format flag
    memcpy(fourCC, &header[80], 4);
    fourCC[4] = '\0';
    unsigned char * buffer;
    unsigned int bufsize;
    
    bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
    buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
    fread(buffer, 1, bufsize, fp);
    fclose(fp);
    

    //Grab the format from the file
    
    unsigned int format;
    format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    
    //Grab the flag from the fourCC variable:
    if (strcmp(fourCC, "DXT1") == 0){format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;}
    else if (strcmp(fourCC, "DXT3") == 0) {format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;}
    else if (strcmp(fourCC, "DXT5") == 0) {format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;}
    else{
        //If it's DX10, we can't use it on skyrim files. return empty.
        free(buffer);
        return 0;
    }


    // Create one OpenGL texture
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
    unsigned int offset = 0;
    
    /* load the mipmaps */
    for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
    {
        unsigned int size = ((width+3)/4)*((height+3)/4)*blockSize;
        glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
                               0, size, buffer + offset);
        
        offset += size;
        width = std::max(width/2, (unsigned int)1);
        height = std::max(height/2, (unsigned int)1);
    }
    //std::cout << textureID << std::endl;
    free(buffer);
    return textureID;
}
