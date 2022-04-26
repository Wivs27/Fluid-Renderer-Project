#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "solver.h"

//rendering code adapted from https://github.com/BlainMaguire/3dfluid

//macro for finding the 1D array index for a corresponding 3D coordinate 
#define IX(i,j,k) ((i)+(N+2)*(j) + (N+2)*(N+2)*(k)) 

//variables to control the size of the simulation as well as the window
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define SIZE 42

//Size of the sim
static int N = SIZE;
//force to be added with the user controls
static float force = 100.0f;
//how much density is added by the user
static float source = 200.0f;
//the alpha value used when rendering the density
static float source_alpha = 0.05; 

//flags used to tell the code when to add force and density
static int addforce[3] = { 0, 0, 0 };
static int addsource = 0;

//global pointer for the simulation
FluidCube* simulation;

//flags used to determine when to draw the density and when to draw velocity
static int dvel = 0;
static int daxis = 0;

//variables for the properties of the window
static int windowID;
static int win_x = WINDOW_WIDTH;
static int win_y = WINDOW_HEIGHT;

//variables used to determine mouse input
static int mouse_down[3];
static int omx, omy, mx, my;

enum {
	PAN = 1,
	ROTATE,
	ZOOM
};

//translation and rotation matrixs used for camera movement
GLfloat trans[3];
GLfloat rot[2];

//function that adds forces and density based on the keyboard input
static void get_force_source() {
	int i, j, k, size = (N + 2) * (N + 2) * (N + 2);

	//checks what flag is active and then adds force in that direction
	//adds force in the x-axis
	if (addforce[0] == 1) {
		//sets the coordinates of where the force will originate
		i = 2,
		j = N / 2;
		k = N / 2;

		//bounds checking to make sure nothing is wrong
		if (i<1 || i>N || j<1 || j>N || k <1 || k>N) {
			return;
		}
		//adds force at that coordinate
		simulation->Vx[IX(i, j, k)] = force;
		//resets flag
		addforce[0] = 0;
	}

	//adds force in the y-axis
	if (addforce[1] == 1) {
		i = N / 2,
		j = 2;
		k = N / 2;

		if (i<1 || i>N || j<1 || j>N || k <1 || k>N) {
			return;
		}
		simulation->Vy[IX(i, j, k)] = force;
		addforce[1] = 0;
	}

	//adds force in the z-axis
	if (addforce[2] == 1) {
		i = N / 2,
		j = N / 2;
		k = 2;

		if (i<1 || i>N || j<1 || j>N || k <1 || k>N) {
			return;
		}
		simulation->Vz[IX(i, j, k)] = force;
		addforce[2] = 0;
	}

	//checks flag to see if user wants to add a source
	if (addsource == 1) {
		//finds coordinate in the middle of the cube
		i = N / 2;
		//adds density at and around the middle
		simulation->density[IX(i, i, i)] += source;
		simulation->density[IX(i - 1, i - 1, i - 1)] += source;
		simulation->density[IX(i + 1, i + 1, i + 1)] += source;
		//resets flag
		addsource = 0;
	}

	return;
}

//function that draws the velocities
static void draw_velocity(void) {
	int i, j, k;
	float x, y, z, h;

	h = 1.0f / N;
	
	//sets the colour and width of the lines representing the velocities
	glColor3f(1.0f, 1.0f, 1.0f);
	glLineWidth(1.0f);

	glBegin(GL_LINES);

	//loops through each coordinate and draws a line from that coordinate 
	//to the coordinate+force vector showing how big each velocity is
	for (i = 1; i <= N; i++) {
		x = (i - 0.5f) * h;
		for (j = 1; j <= N; j++) {
			y = (j - 0.5f) * h;
			for (k = 1; k <= N; k++) {
				z = (k - 0.5f) * h;

				glVertex3f(x, y, z);
				glVertex3f(x + simulation->Vx[IX(i, j, k)], y + simulation->Vy[IX(i, j, k)], z + simulation->Vz[IX(i, j, k)]);
			}
		}
	}

	glEnd();
}

//function used to draw the axis
static void draw_axis() {

	//sets size of lines
	glLineWidth(1.0f);
	//begins the drawing of the lines
	glBegin(GL_LINES);

	//red colour for x-axis
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.25f, 0.f, 0.25f);
	glVertex3f(1.0f, 0.f, 0.25f);

	//green colour for y-axis
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.25f, 0.f, 0.25f);
	glVertex3f(0.25f, 1.0f, 0.25f);

	//blue colour for z-axis
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.25f, 0.f, 0.25f);
	glVertex3f(0.25f, 0.f, 1.0f);

	glEnd();
}

//function used to draw the fluid
static void draw_density(void) {
	int i, j, k;
	float x, y, z, h, d000, d010, d100, d110, d001, d011, d101, d111;

	h = 1.0f / N;

	glBegin(GL_QUADS);

	//for each coordinate
	for (i = 0; i <= N; i++) {
		x = (i - 0.5f) * h;
		for (j = 0; j <= N; j++) {
			y = (j - 0.5f) * h;
			for (k = 0; k <= N; k++) {
				z = (k - 0.5f) * h;
				//gather all the densities in the cube with the original coordinate
				//bottom left corner
				d000 = simulation->density[IX(i, j, k)];
				d010 = simulation->density[IX(i, j + 1, k)];
				d100 = simulation->density[IX(i + 1, j, k)];
				d110 = simulation->density[IX(i + 1, j + 1, k)];

				d001 = simulation->density[IX(i, j, k + 1)];
				d011 = simulation->density[IX(i, j + 1, k + 1)];
				d101 = simulation->density[IX(i + 1, j, k + 1)];
				d111 = simulation->density[IX(i + 1, j + 1, k + 1)];

				//draw density as a cube of quads with the colour intensity 
				//determined by the density
				glColor4f(d111, d111, d111, source_alpha); glVertex3f(x + h, y + h, z + h);
				glColor4f(d011, d011, d011, source_alpha); glVertex3f(x, y + h, z + h);
				glColor4f(d001, d001, d001, source_alpha); glVertex3f(x, y, z + h);
				glColor4f(d101, d101, d101, source_alpha); glVertex3f(x + h, y, z + h);

				glColor4f(d110, d110, d110, source_alpha); glVertex3f(x + h, y + h, z);
				glColor4f(d111, d111, d111, source_alpha); glVertex3f(x + h, y + h, z + h);
				glColor4f(d101, d101, d101, source_alpha); glVertex3f(x + h, y, z + h);
				glColor4f(d100, d100, d100, source_alpha); glVertex3f(x + h, y, z);

				glColor4f(d010, d010, d010, source_alpha); glVertex3f(x, y + h, z);
				glColor4f(d110, d110, d110, source_alpha); glVertex3f(x + h, y + h, z);
				glColor4f(d100, d100, d100, source_alpha); glVertex3f(x + h, y, z);
				glColor4f(d000, d000, d000, source_alpha); glVertex3f(x, y, z);

				glColor4f(d011, d011, d011, source_alpha); glVertex3f(x, y + h, z + h);
				glColor4f(d010, d010, d010, source_alpha); glVertex3f(x, y + h, z);
				glColor4f(d000, d000, d000, source_alpha); glVertex3f(x, y, z);
				glColor4f(d001, d001, d001, source_alpha); glVertex3f(x, y, z + h);

				glColor4f(d100, d100, d100, source_alpha); glVertex3f(x + h, y, z);
				glColor4f(d000, d000, d000, source_alpha); glVertex3f(x, y, z);
				glColor4f(d001, d001, d001, source_alpha); glVertex3f(x, y, z + h);
				glColor4f(d101, d101, d101, source_alpha); glVertex3f(x + h, y, z + h);

				glColor4f(d110, d110, d110, source_alpha); glVertex3f(x + h, y + h, z);
				glColor4f(d010, d010, d010, source_alpha); glVertex3f(x, y + h, z);
				glColor4f(d011, d011, d011, source_alpha); glVertex3f(x, y + h, z + h);
				glColor4f(d111, d111, d111, source_alpha); glVertex3f(x + h, y + h, z + h);
			}
		}
	}

	glEnd();
}

//function that should clamp the rotation to normal values
float clamp(float x) {
	return x > 360.0f ? x - 360.0f : x < -360.0f ? x += 360.0f : x;
}

//updates the translation and rotation vectors based on the mouse movents
static void update(int state, int ox, int nx, int oy, int ny) {

	int dx = ox - nx;
	int dy = ny - oy;

	//state shows which mouse buttons are held down
	switch (state) {
	case ROTATE:
		rot[0] += (dy * 180.0f) / 15000.0f;
		rot[1] -= (dx * 180.0f) / 15000.0f;
		rot[0] = clamp(rot[0]);
		rot[1] = clamp(rot[1]);
		break;
	case PAN:
		trans[0] -= dx / 15000.0f;
		trans[1] -= dy / 15000.0f;
		break;
	case ZOOM:
		trans[2] -= (dx + dy) / 100.0f;
		break;
	}
}

//function that actually advances the simulation
void sim_main(void) {
	//add any forces and density
	get_force_source();
	//step forward in the simulation
	FluidCubeStep(simulation);
}

//function to reset the simulation
void sim_reset() {
	//free any memory being used by the current simulation
	FluidCubeFree(simulation);
	//reallocate the simulation with new memory essential reseting all the values back to 0
	simulation = FluidCubeCreate(N, 0.0f, 0.0f, 0.4f);
}

//function called every display update
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	//set the translation and rotation vectors
	glTranslatef(trans[0], trans[1], trans[2]);
	glRotatef(rot[0], 1.0f, 0.0f, 0.0f);
	glRotatef(rot[1], 0.0f, 1.0f, 0.0f);

	// toggle display modes
	if (dvel) {
		draw_velocity();
	}
	else {
		draw_density();
	}
	if (daxis) {
		draw_axis();
	}

	glEnd();
	glPopMatrix();
	glFlush();
	glutSwapBuffers();
}

//function called when the window is resized by the user
static void reshapeWindow(int width, int height) {
	glutSetWindow(windowID);
	glutReshapeWindow(width, height);

	win_x = width;
	win_y = height;
}

//function that handles all the user input through the keyboard
static void keyboardControls (unsigned char key, int x, int y) {

	switch (key) {
	case 27:		// ESC key
		FluidCubeFree(simulation);
		exit(0);
		break;
	case 'w':       // 'W' key - apply force x-axis
		addforce[1] = 1;
		break;
	case 'd':       // 'D' key - apply force y-axis
		addforce[0] = 1;
		break;
	case 's':       // 'S' key - apply force z-axis
		addforce[2] = 1;
		break;
	case 'x':       // 'X' key - add source at centre
		addsource = 1;
		break;
	case 'c':       // 'C' key - clear simulation
		sim_reset();
		break;
	case 'v':       // 'V' key show velocity
		dvel = !dvel;  // toggle show velocity
		break;
	case 'a':       // 'A' Key draw axis
		daxis = !daxis;  // toggle draw axis
		break;
	}
}

//measures the mouse position and tracks what button was pressed down
static void mouse_func(int button, int state, int x, int y) {
	omx = mx = x;
	omx = my = y;

	mouse_down[button] = state == GLUT_DOWN;
}

//updates variables with the mouses position when moved
//while holding down a button
static void motion_func(int x, int y) {
	omx = mx;
	omy = my;
	mx = x;
	my = y;
}

//function called continuously
static void idle_func(void) {

	// ZOOM state = PAN + ROTATE
	int state = 0;

	if (mouse_down[0]) {
		state |= PAN;
	}

	if (mouse_down[2]) {
		state |= ROTATE;
	}
	//calls update to translate and rotate the view based on mouse state
	update(state, omx, omy, mx, my);

	//calls for the simulation to be ran
	sim_main();

	//lets glut know to re-run the display function next loop
	glutPostRedisplay();
}


int main(int argc, char** argv) {
	
	//creates the simulation struct and allocates all the memory
	simulation = FluidCubeCreate(N, 0.0f, 0.0f, 0.4f);

	//initialise glut and create a blank 800x800 window in the center of the screen
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH)/2-WINDOW_WIDTH/2), (glutGet(GLUT_SCREEN_HEIGHT) / 2 - WINDOW_HEIGHT / 2));
	glutInitWindowSize(win_x, win_y);
	windowID = glutCreateWindow("SC19NW Individual Project: press ESC to quit.");

	//clear both buffers to be blank
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();
	//set all the necessary callback functions
	glutKeyboardFunc(keyboardControls);
	glutMouseFunc(mouse_func);
	glutMotionFunc(motion_func);
	glutReshapeFunc(reshapeWindow);
	glutIdleFunc(idle_func);
	glutDisplayFunc(display);

	//initialise the rotation array
	rot[0] = 30;
	rot[1] = -45;

	//enable blending and alpha testing for rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0);

	//loop until the window is closed
	glutMainLoop();
	//frees memory allocated for the simulation
	FluidCubeFree(simulation);

	return 0;
}