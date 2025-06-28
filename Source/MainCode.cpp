#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

class Brick
{
public:
	float red, green, blue;
	float x, y, width, height;

	BRICKTYPE brick_type;
	ONOFF onoff;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float hh, float rr, float gg, float bb)
	{
		brick_type = bt; x = xx; y = yy; width = ww; height = hh;
		red = rr; green = gg; blue = bb;
		onoff = ON;
	}

	void drawBrick()
	{
		if (onoff == ON)
		{
			double halfW = width / 2;
			double halfH = height / 2;

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfW, y + halfH);
			glVertex2d(x + halfW, y - halfH);
			glVertex2d(x - halfW, y - halfH);
			glVertex2d(x - halfW, y + halfH);

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
	float dx, dy;
	float speed;


	Circle(float xx, float yy, float rr, float dx_, float dy_, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rad;
		red = r;
		green = g;
		blue = b;
		dx = dx_;
		dy = dy_;
		speed = 0.02f;
	}


	void CheckCollision(Brick* brk)
	{
		if (brk->onoff == OFF) return;

		// Generic brick collision check
		if ((x > brk->x - brk->width / 2 && x < brk->x + brk->width / 2) &&
			(y > brk->y - brk->height / 2 && y < brk->y + brk->height / 2))
		{
			if (brk->brick_type == REFLECTIVE) {
				dx = -dx;
				dy = -dy;
			}
			else if (brk->brick_type == DESTRUCTABLE) {
				brk->onoff = OFF;
			}
		}

		// Paddle-specific logic
		
		if (brk->brick_type == REFLECTIVE && brk->y < -0.85f) { // Heuristic to identify paddle
			if ((x > brk->x - brk->width / 2 && x < brk->x + brk->width / 2) &&
				(y - radius < brk->y + brk->height / 2 && y > brk->y)) // From top
			{
				dy = fabs(dy); // Bounce upward
				y = brk->y + brk->height / 2 + radius + 0.001f; // Avoid sticking
			}
		}

	}


	int GetRandomDirection()
	{
		return (rand() % 8) + 1;
	}

	void MoveOneStep()
	{
		x += dx * speed;
		y += dy * speed;

		// Bounce horizontally
		if (x - radius < -1.0f || x + radius > 1.0f)
			dx = -dx;

		// Bounce vertically
		if (y - radius < -1.0f || y + radius > 1.0f)
			dy = -dy;
	}


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
Brick paddle(REFLECTIVE, 0.0f, -0.9f, 0.4f, 0.05f, 1.0f, 1.0f, 1.0f);




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

	vector<Brick> bricks;

	float startX = -0.85f;
	float startY = 0.85f;
	float gap = 0.04f;
	float brickWidths[5] = { 0.1f, 0.12f, 0.14f, 0.12f, 0.1f }; // Predefined sizes

	for (int row = 0; row < 4; row++) {
		float xOffset = startX;
		for (int col = 0; col < 5; col++) {
			float width = brickWidths[col];
			float x = xOffset + width / 2;
			float y = startY - row * (width + gap);
			BRICKTYPE type = (row % 2 == 0) ? REFLECTIVE : DESTRUCTABLE;

			float r = 0.2f * row + 0.2f * (col % 2);
			float g = (type == REFLECTIVE) ? 0.4f + 0.1f * col : 1.0f - 0.1f * row;
			float b = (col % 2 == 0) ? 1.0f - 0.2f * row : 0.5f;

			bricks.emplace_back(type, x, y, width, width, r, g, b);


			xOffset += width + gap;  // Increment by current width
		}
	}


	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		//Movement
		for (int i = 0; i < world.size(); i++)
		{
			for (Brick& b : bricks) {
				world[i].CheckCollision(&b);
			}

			world[i].CheckCollision(&paddle); //  after brick loop

			world[i].MoveOneStep();
			world[i].DrawCircle();

		}

		//1st change
		for (Brick& b : bricks) {
			b.drawBrick();
		}

		// Draw the paddle
		paddle.drawBrick();


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		float r = static_cast<float>(rand() % 1000) / 1000.0f;
		float g = static_cast<float>(rand() % 1000) / 1000.0f;
		float b = static_cast<float>(rand() % 1000) / 1000.0f;

		float angle = static_cast<float>((rand() % 360)) * DEG2RAD;
		float dx = cos(angle);
		float dy = sin(angle);
		Circle B(0.0f, 0.0f, 0.2f, dx, dy, 0.05f, r, g, b);
		world.push_back(B);
	}


	// Paddle movement
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		paddle.x -= 0.03f;

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		paddle.x += 0.03f;

	// Clamp paddle position within screen bounds
	if (paddle.x < -1.0f + paddle.width / 2) paddle.x = -1.0f + paddle.width / 2;
	if (paddle.x > 1.0f - paddle.width / 2) paddle.x = 1.0f - paddle.width / 2;
}
