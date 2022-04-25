#include<iostream>
#include <stdlib.h>
#include<GLFW/glfw3.h>
#include <math.h>

#include "solver.h"


#define IX(x, y, z) ((x) + (y) * N + (z) * N * N)

static int simSize = 256;


int constrain(int a, int min, int max) {
	if (a < min) {
		return min;
	}
	else if (a > max) {
		return max;
	}
	else return a;
}

void mouseInputHandler(float* d, float* u, float* v, GLFWwindow* window) {
	//double mouseX;
	//double mouseY;
	//glfwGetCursorPos(window, &mouseX, &mouseY);
	//int constrainedMouseX = constrain(int(mouseX), 0, simSize);
	//int constrainedMouseY = constrain(int(mouseY), 0, simSize);
	//d[IX(constrainedMouseX,constrainedMouseY)] = 50.0f;
	//d[IX(32, 32)] = 50.0f;
}

void renderSim(FluidCube* cube) {
    glClear(GL_COLOR_BUFFER_BIT);
    float h = 1.0f / simSize, x, y;
    int i, j;
    glBegin(GL_QUADS);
    for (i = 0; i < simSize; i++) {
        x = (i - 0.5f) * h;
        for (j = 0; j < simSize; j++) {
            y = (j - 0.5f) * h;
            glColor3f(1.000, 0.078, 0.576); glVertex2f(x, y);
            glColor3f(1.000, 0.078, 0.576); glVertex2f(x + h, y);
            glColor3f(1.000, 0.078, 0.576); glVertex2f(x + h, y + h);
            glColor3f(1.000, 0.078, 0.576); glVertex2f(x, y + h);
        }
    }
    glEnd();
}

//
// Call back functions for GLFW.
//
void graphicsErrorCallBack(int errorCode, const char* message)
{
    printf("Error with OpenGL/GLFW: code %i, message %s\n", errorCode, message);
}

void keyboardCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Close if the escape key or 'q' is pressed.
    if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
}


int main(){
	// Initialize GLFW
	glfwInit();
	// Create a GLFWwindow object
	GLFWwindow* window = glfwCreateWindow(simSize, simSize, "COMP3931 Project", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

    glfwSetErrorCallback(graphicsErrorCallBack);
    glfwSetKeyCallback(window, keyboardCallBack);
    glfwMakeContextCurrent(window);

    FluidCube* simulation = FluidCubeCreate(simSize, 0, 0.001, 0.2);

	while (!glfwWindowShouldClose(window)){
		glClear(GL_COLOR_BUFFER_BIT);
		//if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		//	mouseInputHandler(dens_prev, u_prev, v_prev, window);
		//}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


    FluidCubeFree(simulation);
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}