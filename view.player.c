#include "view.player.h"
#include <GL/glut.h>
#include <SOIL/SOIL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GLuint playerTexture;

void loadPlayerTexture(const char *imagePath)
{
    playerTexture = SOIL_load_OGL_texture(imagePath, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

    if (!playerTexture)
    {
        printf("Error loading player texture: %s\n", SOIL_last_result());
    }
}

void drawPlayer(float x, float y, float playerWidth, float playerHeight, int frameX, int frameY, int flipHorizontal, int color)
{
    float textureWidth = 1.0f / 6.0f;
    float textureHeight = 1.0f / 10.0f;

    float tx = frameX * textureWidth;
    float ty = frameY * textureHeight;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, playerTexture);

    switch (color)
    {
    case 0:
        glColor3f(1.0f, 0.5f, 0.5f);
        break;
    case 1:
        glColor3f(1.0f, 1.0f, 1.0f);
        break;
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float halfPlayerWidth = playerWidth / 2;
    float halfPlayerHeight = playerHeight / 2;

    glBegin(GL_QUADS);
    if (flipHorizontal)
    {
        glTexCoord2f(tx + textureWidth, ty);
        glVertex2f(x - halfPlayerWidth, y - halfPlayerHeight);
        glTexCoord2f(tx, ty);
        glVertex2f(x + playerWidth - halfPlayerWidth, y - halfPlayerHeight);
        glTexCoord2f(tx, ty + textureHeight);
        glVertex2f(x + playerWidth - halfPlayerWidth, y + playerHeight - halfPlayerHeight);
        glTexCoord2f(tx + textureWidth, ty + textureHeight);
        glVertex2f(x - halfPlayerWidth, y + playerHeight - halfPlayerHeight);
    }
    else
    {
        glTexCoord2f(tx, ty);
        glVertex2f(x - halfPlayerWidth, y - halfPlayerHeight);
        glTexCoord2f(tx + textureWidth, ty);
        glVertex2f(x + playerWidth - halfPlayerWidth, y - halfPlayerHeight);
        glTexCoord2f(tx + textureWidth, ty + textureHeight);
        glVertex2f(x + playerWidth - halfPlayerWidth, y + playerHeight - halfPlayerHeight);
        glTexCoord2f(tx, ty + textureHeight);
        glVertex2f(x - halfPlayerWidth, y + playerHeight - halfPlayerHeight);
    }
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);

    glDisable(GL_TEXTURE_2D);
}
