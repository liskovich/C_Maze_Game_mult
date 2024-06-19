#include "view.objects.h"
#include <GL/glut.h>
#include <SOIL/SOIL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(x) (void)(x)

GLuint wallTexture, pathTexture;

void loadTileTextures(const char* wallImagePath, const char* floorImagePath) {
    wallTexture = SOIL_load_OGL_texture(wallImagePath, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
    pathTexture = SOIL_load_OGL_texture(floorImagePath, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

    long unsigned int i = 0;

    GLuint textures[] = {wallTexture, pathTexture};
    for (i = 0; i < sizeof(textures)/sizeof(textures[0]); i++) {
        if (!textures[i]) {
            printf("Error loading texture: %s\n", SOIL_last_result());
        } else {
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }
}

void drawTile(int x, int y, int type) {
    GLuint textureId = (type == 1) ? wallTexture : pathTexture;
    glBindTexture(GL_TEXTURE_2D, textureId);

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex2f(x, y);
    glTexCoord2f(1.0, 0.0); glVertex2f(x + 1, y);
    glTexCoord2f(1.0, 1.0); glVertex2f(x + 1, y + 1);
    glTexCoord2f(0.0, 1.0); glVertex2f(x, y + 1);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
}