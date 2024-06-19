#ifndef VIEW_PLAYER_H
#define VIEW_PLAYER_H

#include "view.player.h"
#include <GL/glut.h>
#include <SOIL/SOIL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void loadPlayerTexture(const char* imagePath);
void drawPlayer(float x, float y, float playerWidth, float playerHeight, int frameX, int frameY, int flipHorizontal, int color);

#endif
