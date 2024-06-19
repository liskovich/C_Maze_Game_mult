#ifndef VIEW_OBJECTS_H
#define VIEW_OBJECTS_H

#include "view.objects.h"
#include <GL/glut.h>
#include <SOIL/SOIL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void loadCoinTexture(const char* imagePath);
void drawCoin(int32_t id, char type, float x, float y);

#endif
