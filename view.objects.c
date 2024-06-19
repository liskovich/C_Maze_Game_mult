#include "view.objects.h"
#include <GL/glut.h>
#include <SOIL/SOIL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(x) (void)(x)

GLuint coinTexture;

void loadCoinTexture(const char* imagePath) {
    coinTexture = SOIL_load_OGL_texture(imagePath, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

    if (!coinTexture) {
        printf("Error loading coin texture: %s\n", SOIL_last_result());
    } else {
        glBindTexture(GL_TEXTURE_2D, coinTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

void drawCoin(int32_t id, char type, float x, float y) {
    UNUSED(id);
    UNUSED(type);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, coinTexture);

    float coinSize = 0.7f; 
    float shadowOffset = -0.07f;
    float shadowSize = coinSize + 0.1f;

    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x + shadowOffset, y + shadowOffset);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x + shadowSize + shadowOffset, y + shadowOffset);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x + shadowSize + shadowOffset, y + shadowSize + shadowOffset);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x + shadowOffset, y + shadowSize + shadowOffset);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f); 
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x + coinSize, y);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x + coinSize, y + coinSize);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y + coinSize);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

