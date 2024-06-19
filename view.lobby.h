#ifndef VIEW_LOBBY_H
#define VIEW_LOBBY_H

#include "view.tile.h"
#include <GL/glut.h>
#include <SOIL/SOIL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void renderBitmapString(float x, float y, void *font, char *string);
void renderLobbyScreen();
void lobbyInit();

#endif
