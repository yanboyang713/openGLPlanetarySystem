#include <cstdio>		// for C++ i/o
#include <iostream>
#include <string>
#include <cstddef>
#include <cstdlib> // for rand() and srand()
#include <ctime> // for time()
using namespace std;	// to avoid having to use std::

#define GLEW_STATIC		// include GLEW as a static library
#include <GLEW/glew.h>	// include GLEW
#include <GLFW/glfw3.h>	// include GLFW (which includes the OpenGL header)
#include <glm/glm.hpp>	// include GLM (ideally should only use the GLM headers that are actually used)
#include <glm/gtx/transform.hpp>
using namespace glm;	// to avoid having to use glm::

#include "shader.h"
#include "camera.h"
#define PI 3.14159265
#define MAX_SLICES 32
#define MIN_SLICES 32
GLuint orbit_slices = MIN_SLICES;
GLuint g_slices = MIN_SLICES;	// number of circle slices
#define MAX_VERTICES (MAX_SLICES+2)*3	// a triangle fan should have a minimum of 3 vertices
#define CIRCLE_RADIUS 0.5

bool toggleAnimation = false;
Camera g_camera;			// camera

float getRandomNumberBetweenOneToZero();
float getRandomNumberBetweenOneToMinusOne();
int getRandomNumberBetweenThreeToTen();
void filltranslateSizeAndOrbitSize();
void generate_orbit(float, float, float, float);
void generate_ring();

float test = 0.0;
// struct for vertex attributes
struct Vertex
{
	GLfloat position[3];
	GLfloat color[3];
};

// global variables
Vertex g_RingVertices[MAX_VERTICES] = {
	0.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 1.0f,

	0.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 1.0f,
};
Vertex g_orbitVertices[MAX_VERTICES] = 
{
	//position
	0.0f, 0.0f, 0.0f,
	//color
	1.0f, 0.0f, 0.0f,

	//position
	0.0f, 0.0f, 0.0f,
	//color
	1.0f, 0.0f, 0.0f,
};
Vertex g_vertices[] = {
	// vertex 1
	-0.5f, 0.5f, 0.5f,	// position
	1.0f, 0.0f, 1.0f,	// colour
	// vertex 2
	-0.5f, -0.5f, 0.5f,	// position
	1.0f, 0.0f, 0.0f,	// colour
	// vertex 3
	0.5f, 0.5f, 0.5f,	// position
	1.0f, 1.0f, 1.0f,	// colour
	// vertex 4
	0.5f, -0.5f, 0.5f,	// position
	1.0f, 1.0f, 0.0f,	// colour
	// vertex 5
	-0.5f, 0.5f, -0.5f,	// position
	0.0f, 0.0f, 1.0f,	// colour
	// vertex 6
	-0.5f, -0.5f, -0.5f,// position
	0.0f, 0.0f, 0.0f,	// colour
	// vertex 7
	0.5f, 0.5f, -0.5f,	// position
	0.0f, 1.0f, 1.0f,	// colour
	// vertex 8
	0.5f, -0.5f, -0.5f,	// position
	0.0f, 1.0f, 0.0f,	// colour
};

GLuint g_indices[] = {
	0, 1, 2,	// triangle 1
	2, 1, 3,	// triangle 2
	4, 5, 0,	// triangle 3
	0, 5, 1,	// ...
	2, 3, 6,
	6, 3, 7,
	4, 0, 6,
	6, 0, 2,
	1, 5, 3,
	3, 5, 7,
	5, 4, 7,
	7, 4, 6,	// triangle 12
};

GLuint g_IBO = 0;				// index buffer object identifier
GLuint g_VBO[3];				// vertex buffer object identifier
GLuint g_VAO[3];				// vertex array object identifier
GLuint g_shaderProgramID = 0;	// shader program identifier
GLuint g_MVP_Index = 0;			// location in shader
GLuint alfa = 0;
glm::mat4 g_modelMatrix[11];		// object model matrices
glm::mat4 g_viewMatrix;			// view matrix
glm::mat4 g_projectionMatrix;	// projection matrix

enum orbitSpeed {firstPlanetOrbitSpeed, secondPlanetOrbitSpeed, thirdPlanetOrbitSpeed, fourthPlanetOrbitSpeed, moonOrbitSpeed};
float g_orbitSpeed[5] = { 0.75f, -0.65f, 0.55f, -0.45f, 0.65f};
enum orbitSize { firstPlanetOrbitSize, secondPlanetOrbitSize, thirdPlanetOrbitSize, fourthPlanetOrbitSize, moonOrbitSize};
float g_orbitSize[5];
enum translateSize { firstPlanetFirst, firstPlanetSecond, secondPlanetFirst, secondPlanetSecond, thirdPlanetFirst, thirdPlanetSecond, fourthPlanetFirst, fourthPlanetSecond, moonFirst, moonSecond};
float translateAmount[10];

enum rotationSpeed {sunRotationSpeed, secondPlanetRotationSpeed, thirdPlanetRotationSpeed, moonRotationSpeed};
float g_rotationSpeed[4] = { 0.1f, 0.3f, 0.4f, 0.5f };
enum planetSize { firstPlanetSize, secondPlanetSize, thirdPlanetSize, fourthPlanetSize, moonSize};
float g_planetSize[5] = {0.25, 0.65, 0.55, 0.45, 0.35};
enum modle { sunModle, firstPlanetModle, firstPlanetOrbitModle, secondPlanetModle, ringModle, secondPlanetOrbitModle, thirdPlanetModle, thirdPlanetOrbitModle, fourthPlanetModle, fourthPlanetOrbitModle, moonModle};
static void init(GLFWwindow* window)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);	// set clear background colour
	generate_orbit(0.0, 0.0, 0.0, 1.0);
	filltranslateSizeAndOrbitSize();
	generate_ring();
	glEnable(GL_DEPTH_TEST);	// enable depth buffer test

	// create and compile our GLSL program from the shader files
	g_shaderProgramID = loadShaders("MVP_VS.vert", "ColorFS.frag");

	// find the location of shader variables
	GLuint positionIndex = glGetAttribLocation(g_shaderProgramID, "aPosition");
	GLuint colorIndex = glGetAttribLocation(g_shaderProgramID, "aColor");
	alfa = glGetUniformLocation(g_shaderProgramID, "alfa");
	g_MVP_Index = glGetUniformLocation(g_shaderProgramID, "uModelViewProjectionMatrix");

	// initialise model matrix to the identity matrix

	g_modelMatrix[sunModle] = glm::mat4(1.0f);
	g_modelMatrix[firstPlanetModle] = glm::mat4(1.0f);
	g_modelMatrix[firstPlanetOrbitModle] = glm::mat4(1.0f);
	g_modelMatrix[secondPlanetModle] = glm::mat4(1.0f);
	g_modelMatrix[ringModle] = glm::mat4(1.0f);
	g_modelMatrix[secondPlanetOrbitModle] = glm::mat4(1.0f);
	g_modelMatrix[thirdPlanetModle] = glm::mat4(1.0f);
	g_modelMatrix[moonModle] = glm::mat4(1.0f);
	g_modelMatrix[thirdPlanetOrbitModle] = glm::mat4(1.0f);
	g_modelMatrix[fourthPlanetModle] = glm::mat4(1.0f);
	g_modelMatrix[fourthPlanetOrbitModle] = glm::mat4(1.0f);

	// set camera's view matrix
	g_camera.setViewMatrix(glm::vec3(0, 0, 5), glm::vec3(0, 0, 4), glm::vec3(0, 1, 0));

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspectRatio = static_cast<float>(width) / height;

	// set camera's projection matrix
	g_camera.setProjectionMatrix(glm::perspective(45.0f, aspectRatio, 0.1f, 1000.0f));
	
	// generate identifier for VBO and copy data to GPU
	glGenBuffers(3, g_VBO);
	//cube
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertices), g_vertices, GL_STATIC_DRAW);
	//orbit
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_orbitVertices), g_orbitVertices, GL_DYNAMIC_DRAW);
	//ring
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_RingVertices), g_RingVertices, GL_DYNAMIC_DRAW);

	// generate identifier for IBO and copy data to GPU
	glGenBuffers(1, &g_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_indices), g_indices, GL_STATIC_DRAW);

	// generate identifiers for VAO
	glGenVertexArrays(3, g_VAO);

	// create VAO and specify VBO data
	//cube
	glBindVertexArray(g_VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);

	// interleaved attributes
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));

	glEnableVertexAttribArray(positionIndex);	// enable vertex attributes
	glEnableVertexAttribArray(colorIndex);

	//orbit
	glBindVertexArray(g_VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[1]);
	// interleaved attributes
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));

	glEnableVertexAttribArray(positionIndex);	// enable vertex attributes
	glEnableVertexAttribArray(colorIndex);

	//ring
	glBindVertexArray(g_VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[2]);
	// interleaved attributes
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));

	glEnableVertexAttribArray(positionIndex);	// enable vertex attributes
	glEnableVertexAttribArray(colorIndex);
}

// function used to update the scene
static void update_scene(GLFWwindow* window, double frameTime)
{
	// static variables for rotation angles
	static float orbitAngle[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	static float rotationAngle[4] = { 0.0f, 0.0f, 0.0f, 0.0f};

	// update rotation angles
	for (int i = 0; i < 5; i++)
	{
		orbitAngle[i] += g_orbitSpeed[i] * frameTime;
	}
	for (int i = 0; i < 4; i++)
		rotationAngle[i] += g_rotationSpeed[0] * frameTime;


	// update model matrix
	//sun
	g_modelMatrix[sunModle] = glm::rotate(rotationAngle[sunRotationSpeed], glm::vec3(0.0f, 1.0f, 0.0f));
	//first planet
	g_modelMatrix[firstPlanetModle] = glm::translate(glm::vec3(translateAmount[firstPlanetFirst], 0.0f, 0.0f))
		* glm::rotate(orbitAngle[firstPlanetOrbitSpeed], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(translateAmount[firstPlanetSecond], 0.0f, 0.0f))
		* glm::rotate(45.0f, glm::vec3(1.0f, 0.0f, 1.0f))
		* glm::scale(glm::vec3(g_planetSize[firstPlanetSize], g_planetSize[firstPlanetSize], g_planetSize[firstPlanetSize]));

	//first planet orbit
	g_modelMatrix[firstPlanetOrbitModle] = glm::translate(glm::vec3(translateAmount[firstPlanetFirst], 0.0f, 0.0f))
		* glm::rotate(1.57f, glm::vec3(1.0f, 0.0f, 0.0f))
	* glm::scale(glm::vec3(g_orbitSize[firstPlanetOrbitSize], g_orbitSize[firstPlanetOrbitSize], g_orbitSize[firstPlanetOrbitSize]));
	
	//second planet
	g_modelMatrix[secondPlanetModle] = glm::translate(glm::vec3(translateAmount[secondPlanetFirst], 0.0f, 0.0f))
		* glm::rotate(orbitAngle[secondPlanetOrbitSpeed], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(translateAmount[secondPlanetSecond], 0.0f, 0.0f))
		* glm::scale(glm::vec3(g_planetSize[secondPlanetSize], g_planetSize[secondPlanetSize], g_planetSize[secondPlanetSize]));
	
	//ring
	g_modelMatrix[ringModle] = g_modelMatrix[secondPlanetModle]
		* glm::rotate(1.57f, glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::scale(glm::vec3(g_planetSize[secondPlanetSize] + 2, g_planetSize[secondPlanetSize] + 2, g_planetSize[secondPlanetSize] + 2));
	//second planet orbit
	g_modelMatrix[secondPlanetOrbitModle] = glm::translate(glm::vec3(translateAmount[secondPlanetFirst], 0.0f, 0.0f))
		* glm::rotate(1.57f, glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::scale(glm::vec3(g_orbitSize[secondPlanetOrbitSize], g_orbitSize[secondPlanetOrbitSize], g_orbitSize[secondPlanetOrbitSize]));
	
	//third Planet
	g_modelMatrix[thirdPlanetModle] = glm::translate(glm::vec3(translateAmount[thirdPlanetFirst], 0.0f, 0.0f))
		* glm::rotate(orbitAngle[thirdPlanetOrbitSpeed], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(translateAmount[thirdPlanetSecond], 0.0f, 0.0f))
		* glm::rotate(rotationAngle[thirdPlanetRotationSpeed], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::vec3(g_planetSize[thirdPlanetSize], g_planetSize[thirdPlanetSize], g_planetSize[thirdPlanetSize]));
	//moon
	g_modelMatrix[moonModle] = g_modelMatrix[thirdPlanetModle]
		* glm::translate(glm::vec3(translateAmount[moonFirst], 0.0f, 0.0f))
		* glm::rotate(orbitAngle[moonOrbitSpeed], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(translateAmount[moonSecond], 0.0f, 0.0f))
		* glm::scale(glm::vec3(g_planetSize[moonSize], g_planetSize[moonSize], g_planetSize[moonSize]));
	//third Planet orbit
	g_modelMatrix[thirdPlanetOrbitModle] = glm::translate(glm::vec3(translateAmount[thirdPlanetFirst], 0.0f, 0.0f))
		* glm::rotate(1.57f, glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::scale(glm::vec3(g_orbitSize[thirdPlanetOrbitSize], g_orbitSize[thirdPlanetOrbitSize], g_orbitSize[thirdPlanetOrbitSize]));

	//fourth Planet 
	g_modelMatrix[fourthPlanetModle] = glm::translate(glm::vec3(translateAmount[fourthPlanetFirst], 0.0f, 0.0f))
		* glm::rotate(orbitAngle[fourthPlanetOrbitSpeed], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(translateAmount[fourthPlanetSecond], 0.0f, 0.0f))
		* glm::scale(glm::vec3(g_planetSize[fourthPlanetSize], g_planetSize[fourthPlanetSize], g_planetSize[fourthPlanetSize]));
	//fourth Planet orbit
	g_modelMatrix[fourthPlanetOrbitModle] = glm::translate(glm::vec3(translateAmount[fourthPlanetFirst], 0.0f, 0.0f))
		* glm::rotate(1.57f, glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::scale(glm::vec3(g_orbitSize[fourthPlanetOrbitSize], g_orbitSize[fourthPlanetOrbitSize], g_orbitSize[fourthPlanetOrbitSize]));
}

// function used to render the scene
static void render_scene(float count)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// clear colour buffer and depth buffer

	glUseProgram(g_shaderProgramID);	// use the shaders associated with the shader program

	glBindVertexArray(g_VAO[0]);		// make VAO active
// sun
	glUniform1f(alfa, 1);
	glm::mat4 MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[sunModle];
	// set uniform model transformation matrix
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

// first planet
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[firstPlanetModle];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

// first planet orbit
	glBindVertexArray(g_VAO[1]);
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[firstPlanetOrbitModle];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_LINE_LOOP, 0, orbit_slices);	// display the vertices based on their indices and primitive type

													
// second planet
	glBindVertexArray(g_VAO[0]);		// make VAO active
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[secondPlanetModle];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type
//ring
	glBindVertexArray(g_VAO[2]);
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[ringModle];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, g_slices +4);	// display the vertices based on their indices and primitive type
// second planet orbit
	glBindVertexArray(g_VAO[1]);
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[secondPlanetOrbitModle];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_LINE_LOOP, 0, orbit_slices);	// display the vertices based on their indices and primitive type


	
// third planet
	glBindVertexArray(g_VAO[0]);		// make VAO active
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[thirdPlanetModle];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type
//moon
	glBindVertexArray(g_VAO[0]);		// make VAO active
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[moonModle];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type
// third planet orbit
	glBindVertexArray(g_VAO[1]);
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[thirdPlanetOrbitModle];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_LINE_LOOP, 0, orbit_slices);	// display the vertices based on their indices and primitive type

//fourth Planet 
	glBindVertexArray(g_VAO[0]);		// make VAO active
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[fourthPlanetModle];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniform1f(alfa, count);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type
	glUniform1f(alfa, 1);
//fourth Planet orbit
	glBindVertexArray(g_VAO[1]);
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[fourthPlanetOrbitModle];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_LINE_LOOP, 0, orbit_slices);	// display the vertices based on their indices and primitive type

	glFlush();	// flush the pipeline
}

// key press or release callback function
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// quit if the ESCAPE key was press
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		if (toggleAnimation == false)
		{
			for (int i = 0; i < 5; i++)
				g_orbitSpeed[i] = 0.0;
			for (int i = 0; i < 4; i++)
				g_rotationSpeed[i] = 0.0;
			toggleAnimation = true;
		}
		else
		{
			srand(time(NULL));
			for (int i = 0; i < 5; i++)
				g_orbitSpeed[i] = getRandomNumberBetweenOneToMinusOne();
			for (int i = 0; i < 4; i++)
				g_rotationSpeed[i] = getRandomNumberBetweenOneToMinusOne();
			toggleAnimation = false;
		}
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		srand(time(NULL));
		for (int i = 0; i < 5; i++)
			g_orbitSpeed[i] = getRandomNumberBetweenOneToMinusOne();
		for (int i = 0; i < 4; i++)
			g_rotationSpeed[i] = getRandomNumberBetweenOneToMinusOne();
		for (int i = 0; i < 5; i++)
			g_planetSize[i] = getRandomNumberBetweenOneToZero();
		filltranslateSizeAndOrbitSize();
	}
	else if (key == GLFW_KEY_1 && action == GLFW_PRESS)//overView
	{
		g_camera.setYaw(0.03);
		g_camera.setPitch(-0.79);
		g_camera.setViewMatrix(glm::vec3(0, 100, 100), glm::vec3(0, 0, 0), glm::vec3(0, 1, -1));
	}
	else if (key == GLFW_KEY_2 && action == GLFW_PRESS)//top-down view
	{
		g_camera.setYaw(-0.04);
		g_camera.setPitch(-1.59);
		g_camera.setViewMatrix(glm::vec3(0, 100, 0), glm::vec3(0, 0, 0), glm::vec3(1, 0, 0));
	}

	
}

// mouse movement callback function
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// variables to store mouse cursor coordinates
	static double previous_xpos = xpos;
	static double previous_ypos = ypos;
	double delta_x = xpos - previous_xpos;
	double delta_y = ypos - previous_ypos;

	// pass mouse movement to camera class
	g_camera.updateYaw(delta_x);
	g_camera.updatePitch(delta_y);

	// update previous mouse coordinates
	previous_xpos = xpos;
	previous_ypos = ypos;
}

// mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

}

// error callback function
static void error_callback(int error, const char* description)
{
	cerr << description << endl;	// output error description
}

int main(void)
{
	GLFWwindow* window = NULL;	// pointer to a GLFW window handle

	glfwSetErrorCallback(error_callback);	// set error callback function

	// initialise GLFW
	if (!glfwInit())
	{
		// if failed to initialise GLFW
		exit(EXIT_FAILURE);
	}

	// minimum OpenGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// create a window and its OpenGL context
	window = glfwCreateWindow(800, 600, "DemoCode", NULL, NULL);

	// if failed to create window
	if (window == NULL)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);	// set window context as the current context
	glfwSwapInterval(1);			// swap buffer interval

	// initialise GLEW
	if (glewInit() != GLEW_OK)
	{
		// if failed to initialise GLEW
		cerr << "GLEW initialisation failed" << endl;
		exit(EXIT_FAILURE);
	}

	// set key callback function
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// use sticky mode to avoid missing state changes from polling
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// use mouse to move camera, hence use disable cursor mode
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// initialise rendering states
	init(window);

	// variables for simple time management
	float lastUpdateTime = glfwGetTime();
	double elapsedTime = lastUpdateTime;	// time elapsed since last update
	double frameTime = 0.0f;				// frame time
	int frameCount = 0;						// number of frames since last update
	float count = 0.0;
	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		g_camera.update(window);	// update cameras
		update_scene(window, frameTime);		// update the scene
		render_scene(count);		// render the scene

		glfwSwapBuffers(window);	// swap buffers
		glfwPollEvents();			// poll for events

		frameCount++;
		elapsedTime = glfwGetTime() - lastUpdateTime;	// current time - last update time
		// only update if more than 0.02 seconds since last update
		if (elapsedTime >= 1.0f)
		{
			frameTime = 1.0f / frameCount;	// calculate frame time

			string str = "FPS = " + to_string(frameCount) + "; FT = " + to_string(frameTime);

			glfwSetWindowTitle(window, str.c_str());	// update window title

			frameCount = 0;					// reset frame count
			lastUpdateTime += elapsedTime;	// update last update time
		}
		count += 0.01;
		if (count >= 1)
			count = 0.0;
	}

	// clean up
	glDeleteProgram(g_shaderProgramID);
	glDeleteBuffers(1, &g_IBO);
	glDeleteBuffers(3, g_VBO);
	glDeleteVertexArrays(3, g_VAO);

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}
void generate_orbit(float centreX, float centreY, float centreZ, float circleRadius)
{
	float angle = PI * 2 / static_cast<float>(orbit_slices);	// used to generate x and y coordinates
	// generate vertex
	for (int i = 0; i < orbit_slices +4; i++)
	{
		// multiply by 3 because a vertex has x, y, z coordinates

		g_orbitVertices[i].position[0] = circleRadius * cos(angle) + centreX;
		g_orbitVertices[i].color[0] = 1.0f;
		g_orbitVertices[i].position[1] = circleRadius * sin(angle) + centreY;
		g_orbitVertices[i].color[1] = 0.0f;
		g_orbitVertices[i].position[2] = 0.0f + centreZ;
		g_orbitVertices[i].color[2] = 0.0f;

		// update to next angle
		angle += PI * 2 / static_cast<float>(orbit_slices);
	}
	return;
}
void generate_ring()
{
	float angle = PI * 2 / static_cast<float>(g_slices);	// used to generate x and y coordinates
	int index = 0;	// vertex index

	g_RingVertices[2].position[0] = CIRCLE_RADIUS ;	// set x coordinate of vertex 1
	srand((unsigned)time(NULL));
	// generate vertex coordinates for triangle fan
	for (int i = 3; i < g_slices + 4; i++)
	{
		// multiply by 3 because a vertex has x, y, z coordinates
		
		g_RingVertices[i].position[0] = CIRCLE_RADIUS * cos(angle);
		g_RingVertices[i].color[0] = getRandomNumberBetweenOneToZero();
		g_RingVertices[i].position[1] = CIRCLE_RADIUS * sin(angle);
		g_RingVertices[i].color[1] = getRandomNumberBetweenOneToZero();
		g_RingVertices[i].position[2] = 0.0f;
		g_RingVertices[i].color[2] = getRandomNumberBetweenOneToZero();

		// update to next angle
		angle += PI * 2 / static_cast<float>(g_slices);
	}
}
float getRandomNumberBetweenOneToZero()
{
	int numberOne = rand() % 100 + 1;
	if (numberOne == 100)
		numberOne--;
	return static_cast<float>(numberOne) / 100;
}
float getRandomNumberBetweenOneToMinusOne()
{
	float numberOne = getRandomNumberBetweenOneToZero();
	int numberTwo = rand() % 2;
	if (numberTwo == 1)
		numberOne *= -1;
	return numberOne;
}
int getRandomNumberBetweenThreeToTen()
{
	int number = rand() % 5 + 3;
	return number;
}
void filltranslateSizeAndOrbitSize()
{
	srand(time(NULL));
	translateAmount[0] = getRandomNumberBetweenThreeToTen();
	translateAmount[1] = (getRandomNumberBetweenThreeToTen() * -1) - translateAmount[0];
	for (int i = 2; i < 8; i++)
	{
		translateAmount[i] = translateAmount[i-2] + getRandomNumberBetweenThreeToTen();
		i++;
		translateAmount[i] = translateAmount[i - 2] - getRandomNumberBetweenThreeToTen() - translateAmount[i - 1];
	}
	int count = 0;
	for (int i = 0; i < 4; i++)
	{
		g_orbitSize[i] = -1 * translateAmount[count+1];
		count = count + 2;
	}
	//moon
	translateAmount[moonSecond] = -1 * getRandomNumberBetweenThreeToTen();
	translateAmount[moonFirst] = getRandomNumberBetweenThreeToTen();
	g_orbitSize[moonOrbitSize] = -1 * translateAmount[moonSecond];
	
	return;
}