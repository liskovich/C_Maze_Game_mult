#include <GL/glut.h>
#include <SOIL/SOIL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "view.player.h"
#include "view.objects.h"
#include "view.tile.h"

#define UNUSED(x) (void)(x)

int mazeWidth = 15;
int mazeHeight = 15;
int maze[15][15] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,0,1,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,1,0,1},
    {1,1,1,1,1,1,1,1,1,0,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,1,0,1,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,0,1,1,1,1,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,0,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};


float playerX = 0.0f;
float playerY = 0.0f;

int displayX = 0;
int displayY = 9;

int flipHorizontal = 0;

void initializeCoins() {
    drawCoin(1, 'c', 1.0, 1.0);
    drawCoin(2, 'c', 6.0, 11.0); 
    drawCoin(3, 'c', 11.0, 7.5); 
}

void initializeTextures() {
    loadPlayerTexture("./sprites/characters/player.png");
    loadCoinTexture("./sprites/objects/coin.png");

    char* wallTexture = "./sprites/tiles/wall.png";
    char* floorTexture = "./sprites/tiles/floor.png";
    loadTileTextures(wallTexture, floorTexture);
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); 

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, mazeWidth, 0.0, mazeHeight);
}


void drawGrid() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    
    for (int i = 0; i < mazeHeight; i++) {
        for (int j = 0; j < mazeWidth; j++) {
            drawTile(j, mazeHeight - i - 1, maze[i][j]);
        }
    }

    glDisable(GL_TEXTURE_2D);

    glFlush();
}

void handleMovement(unsigned char key, int x, int y) {
    /*
    send
    */
    /*
    * Unused
    */
    UNUSED(x);
    UNUSED(y);

    float player_speed = 0.3f;
    float newPlayerX = playerX;
    float newPlayerY = playerY;

    switch (key) {
        case 'w': /* Up */
            newPlayerY += player_speed;
            break;
        case 's': /* Down */
            newPlayerY -= player_speed;
            break;
        case 'a': /* Left */
            newPlayerX -= player_speed;
            break;
        case 'd': /* Right */
            newPlayerX += player_speed;
            break;
    }

    /* Setting map borders */
    if (newPlayerX >= -1 && newPlayerX <= mazeWidth - 1 && newPlayerY >= -0.5 && newPlayerY <= mazeHeight - 0.5) {
        playerX = newPlayerX;
        playerY = newPlayerY;
    }

    switch (key) {
        case 'w':
            flipHorizontal = 0;
            displayY = 7; /* Direction up */
            break;
        case 's':
            flipHorizontal = 0;
            displayY = 9; /* Direction down */
            break;
        case 'a':
            flipHorizontal = 1;
            displayY = 8; /* Direction left */
            break;
        case 'd':
            flipHorizontal = 0;
            displayY = 8; /* Direction right */
            break;
    }

    glutPostRedisplay();
}

/* Reshape function to set game to always be square */
void reshape(int width, int height) {
    int size = (width > height) ? height : width; 

    int x = (width - size) / 2;
    int y = (height - size) / 2;

    glViewport(x, y, size, size);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, mazeWidth, 0.0, mazeHeight); 
    glMatrixMode(GL_MODELVIEW);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT); 

    drawGrid();

    initializeCoins(); /* Draw coins */

    drawPlayer(playerX, playerY, 2.0f, 2.0f, displayX, displayY, flipHorizontal, 0); /* Draw player */
    // drawPlayer(playerX, playerY, 2.5f, 2.5f, displayX, displayY, flipHorizontal, 1); /* Draw player */

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(800, 800);
    glutCreateWindow("2-Color Tile Maze");

    init();
    initializeTextures();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleMovement);

    glutMainLoop();
    return 0;
}
