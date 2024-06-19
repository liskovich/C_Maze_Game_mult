#ifndef VIEW_TILE_H
#define VIEW_TILE_H

#include "view.tile.h"
#include <GL/glut.h>
#include <SOIL/SOIL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void loadTileTextures(const char* wallImagePath, const char* floorImagePath);
void drawTile(int x, int y, int type);

#endif
