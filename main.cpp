#include<iostream>
#include <stdlib.h>
#include<GLFW/glfw3.h>
#include "solver.h"

#define IX(i,j) ((i)+(N+2)*(j))

//global variables used in the sim
static int N = 128, Scale = 5;
static float dt = 0.1f, diff = 0.0f, visc = 0.0f;

static float* u, * v, * u_prev, * v_prev;
static float* dens, * dens_prev;

// Constant variables to set the size of the window
const unsigned int width = N * Scale;
const unsigned int height = N * Scale;

//allocates memory for all the arrays used
int allocateMemory() {
	int size = (N + 2) * (N + 2);
	u = (float*)calloc(size, sizeof(float));
	v = (float*)calloc(size, sizeof(float));
	u_prev = (float*)calloc(size, sizeof(float));
	v_prev = (float*)calloc(size, sizeof(float));
	dens = (float*)calloc(size, sizeof(float));
	dens_prev = (float*)calloc(size, sizeof(float));
	//error checking to ensure memory was allocated
	if (!u || !v || !u_prev || !v_prev || !dens || !dens_prev) {
		printf("Program failed to allocate memory. \n");
		return(0);
	}
	return(1);
}

void freeMemory() {
	free(u);
	free(v);
	free(u_prev);
	free(v_prev);
	free(dens);
	free(dens_prev);
}

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
	double mouseX;
	double mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	int constrainedMouseX = constrain(int(mouseX) / Scale, 0, N);
	int constrainedMouseY = constrain(int(mouseY) / Scale, 0, N);
	d[IX(constrainedMouseX, constrainedMouseY)] = 100.0f;
}

void renderSim() {
	glClear(GL_COLOR_BUFFER_BIT);
	int i, j;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			glColor3f(0 + dens[IX(i, j)], 0 + dens[IX(i, j)], 0 + dens[IX(i, j)]);
			printf("%f \n", dens[IX(i, j)]);
			glBegin(GL_POLYGON);
			glVertex3f(-1.0f + i * Scale, -1.0f + j * Scale, 0.0f);
			glVertex3f(-1.0f + (i + 1) * Scale, -1.0f + j * Scale, 0.0f);
			glVertex3f(-1.0f + (i + 1) * Scale, -1.0f + (j + 1) * Scale, 0.0f);
			glVertex3f(-1.0f + i * Scale, -1.0f + (j + 1) * Scale, 0.0f);
			glEnd();
		}
	}
}

int main(){
	// Initialize GLFW
	glfwInit();
	// Create a GLFWwindow object
	GLFWwindow* window = glfwCreateWindow(width, height, "COMP3931 Project", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);

	if (!allocateMemory()) {
		return(-1);
	}
	int i;
	for (i = 0; i < 1700000; i++) {
		printf("%d  =  %f\n", i, dens_prev[i]);
	}
	double mouseX, mouseY;
	// Main while loop
	while (!glfwWindowShouldClose(window)){
		glClear(GL_COLOR_BUFFER_BIT);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			mouseInputHandler(dens_prev, u_prev, v_prev, window);
		}
		vel_step(N, u, v, u_prev, v_prev, visc, dt);
		dens_step(N, dens, dens_prev, u, v, diff, dt);
		renderSim();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	freeMemory();
	return 0;
}