#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int windowWidth = 800;
const int windowHeight = 800;

void renderBitmapString(float x, float y, void *font, char *string) {
    char *c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void renderLobbyScreen() {
    glColor3f(1.0, 1.0, 1.0); /* Text color */

    /* Text */
    renderBitmapString(windowWidth / 2 - 50, windowHeight / 2 - 10, GLUT_BITMAP_TIMES_ROMAN_24, "Superior maze coin game");
    renderBitmapString(50, windowHeight - 100, GLUT_BITMAP_9_BY_15, "press 'C' to connect");
    renderBitmapString(50, windowHeight - 50, GLUT_BITMAP_9_BY_15, "press 'Esc' to exit");
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    renderLobbyScreen();

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'c':
            printf("Connect action triggered\n");
            break;
        case 27:  // ESC key
            exit(0);
            break;
    }
}
/* All of the  important lobby functions in one init */
void lobbyInit() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, windowWidth, windowHeight, 0.0);
    glMatrixMode(GL_MODELVIEW);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("OpenGL Lobby Screen");

    lobbyInit();
    glutMainLoop();

    return 0;
}
