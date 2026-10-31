#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>
#include <cmath>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

class Brick
{
public:
	// RGB color values
	float red, green, blue;

	// Position & size
	float x, y, width;

	// Brick behavior and state
	BRICKTYPE brick_type;
	ONOFF onoff;

	// Number of hits before destruction
	int hitsRemaining;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb)
	{
		brick_type = bt; x = xx; y = yy, width = ww; red = rr, green = gg, blue = bb;
		onoff = ON;
		hitsRemaining = 3;
	};

	void drawBrick()
	{
		if (onoff == ON)
		{
			double halfside = width / 2;

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);

			glEnd();
		}
	}
};


class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.03;
	int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left

	Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
	}

	// Check collision between a circle and a brick
	void CheckCollision(Brick* brk)
	{
		// Reflective bricks redirect the circle
		if (brk->brick_type == REFLECTIVE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				direction = GetRandomDirection();
				x = x + 0.03;
				y = y + 0.04;
			}
		}

		// Destructable bricks lose health when hit
		else if (brk->brick_type == DESTRUCTABLE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				brk->hitsRemaining--;

				brk->red -= 0.2f;
				brk->green -= 0.2f;

				if (brk->hitsRemaining <= 0) {

					brk->onoff = OFF;
				}
				
			}
		}
	}

	// Generate a random movement direction
	int GetRandomDirection()
	{
		return (rand() % 8) + 1;
	}

	// Detect collision between two circles
	bool CheckCircleCollision(Circle& other) {

		float dx = x - other.x;
		float dy = y - other.y;

		float distance = sqrt(dx * dx + dy * dy);

		return distance < (radius + other.radius);

	}

	// Move the circle and apply wall bounce physics
	void MoveOneStep()
	{
		// Prevent circles from moving too fast
		if (speed > 0.08f) {

			speed = 0.08f;
		}

		// Move upward
		if (direction == 1 || direction == 5 || direction == 6)  // up
		{
			if (y > -1 + radius)
			{
				y -= speed;
			}
			else
			{
				//direction = GetRandomDirection();
				direction = 3;
				speed += 0.002;
			}
		}

		// Move right
		if (direction == 2 || direction == 5 || direction == 7)  // right
		{
			if (x < 1 - radius)
			{
				x += speed;
			}
			else
			{
				//direction = GetRandomDirection();
				direction = 4;
				speed += 0.002;
			}
		}

		// Move downward
		if (direction == 3 || direction == 7 || direction == 8)  // down
		{
			if (y < 1 - radius) {
				y += speed;
			}
			else
			{
				//direction = GetRandomDirection();
				direction = 1;
				speed += 0.002;
			}
		}

		// Move left
		if (direction == 4 || direction == 6 || direction == 8)  // left
		{
			if (x > -1 + radius) {
				x -= speed;
			}
			else
			{
				//direction = GetRandomDirection();
				direction = 2;
				speed += 0.002;
			}
		}
	}

	// Draw circle on the screen
	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};


vector<Circle> world;


int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "8-2 Assignment", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	/*
	Brick brick(REFLECTIVE, 0.5, -0.33, 0.2, 1, 1, 0);
	Brick brick2(DESTRUCTABLE, -0.5, 0.33, 0.2, 0, 1, 0);
	Brick brick3(DESTRUCTABLE, -0.5, -0.33, 0.2, 0, 1, 1);
	Brick brick4(REFLECTIVE, 0, 0, 0.2, 1, 0.5, 0.5);
	*/

	// Create a grid of reflective and destructible bricks
	vector<Brick> bricks;

	// Generate brick layout
	for (int row = 0; row < 3; row++)
	{
		for (int col = 0; col < 5; col++)
		{
			float x = -0.8f + (col * 0.4f);
			float y = -0.5f + (row * 0.4f);

			if ((row + col) % 2 == 0) {

				bricks.push_back(

					Brick(REFLECTIVE, x, y, 0.15f, 1.0f, 0.3f, 0.3f));
			}

			else {

				bricks.push_back(
					Brick(DESTRUCTABLE, x, y, 0.15f, 0.2f, 0.5f, 1.0f));
			}



		}


	}


	// Main game loop
	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		// Process keyboard input 
		processInput(window);

		//Update circle movement and brick collisions
		for (int i = 0; i < world.size(); i++)
		{
			/*
			world[i].CheckCollision(&brick);
			world[i].CheckCollision(&brick2);
			world[i].CheckCollision(&brick3);
			world[i].CheckCollision(&brick4);
			world[i].MoveOneStep();
			world[i].DrawCircle();
			*/

			for (int j = 0; j < bricks.size(); j++) {

				world[i].CheckCollision(&bricks[j]);
			}

			world[i].MoveOneStep();
			world[i].DrawCircle();
		}

		/*
		brick.drawBrick();
		brick2.drawBrick();
		brick3.drawBrick();
		brick4.drawBrick();
		*/

		// Check collisions between circles
		for (int i = 0; i < world.size(); i++)
		{
			for (int j = i + 1; j < world.size(); j++)
			{

				if (world[i].CheckCircleCollision(world[j])) {

					world[i].red = 1.0f;
					world[i].green = 1.0f;
					world[i].blue = 0.0f;

					world[j].red = 1.0f;
					world[j].green = 1.0f;
					world[j].blue = 0.0f;

				}
				


			}



		}

		// Draw all active bricks
		for (int i = 0; i < bricks.size(); i++) {

			bricks[i].drawBrick();
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

// Processes keyboard input function/method
void processInput(GLFWwindow* window)
{
	// Esc to close out of the program
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Spacebar creates a new circle with random positions
	// color, and direction
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		/*
		double r, g, b;
		r = rand() / 10000;
		g = rand() / 10000;
		b = rand() / 10000;
		*/

		float r = (float)rand() / RAND_MAX;
		float g = (float)rand() / RAND_MAX;
		float b = (float)rand() / RAND_MAX;

		float startX = ((float)rand() / RAND_MAX) * 1.6f - 0.8f;
		float startY = ((float)rand() / RAND_MAX) * 1.6f - 0.8f;

		Circle B(startX, startY, 2, rand() % 8 + 1, 0.05f, r, g, b);


		world.push_back(B);
	}
}