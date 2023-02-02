#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "EW/Shader.h"
#include "EW/ShapeGen.h"

#include <math.h>

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);

float lastFrameTime;
float deltaTime;

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

double prevMouseX;
double prevMouseY;
bool firstMouseInput = false;

/* Button to lock / unlock mouse
* 1 = right, 2 = middle
* Mouse will start locked. Unlock it to use UI
* */
const int MOUSE_TOGGLE_BUTTON = 1;
const float MOUSE_SENSITIVITY = 0.1f;

glm::vec3 bgColor = glm::vec3(0);
float exampleSliderFloat = 0.0f;

namespace matrises
{
	glm::mat4 vec3 transform, rotation, size;

	glm::mat4 scale(glm::vec3 s)
	{
		return glm::mat4(
			s.x, 0,  0,   0,
			0,  s.y, 0,   0,
			0,  0,  s.z,  0,
			0,  0,   0,   1
		);
	}

	glm::mat4 rotate(glm::vec3 r)
	{
		glm::mat4 rotateX(
			1, 0, 0, 0,
			0, cos(r.x), sin(r.x), 0,
			0, -sin(r.x), cos(r.x), 0,
			0, 0, 0, 1
		);
		glm::mat4 rotateY(
			cos(r.y), 0, sin(r.y), 0,
			0, 1, 0, 0,
			sin(r.y), 0, cos(r.y), 0,
			0, 0, 0, 1
		);
		glm::mat4 rotateZ(
			cos(r.z), sin(r.z), 0, 0,
			-sin(r.z), cos(r.z), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		);

		return rotateX * rotateY * rotateZ;
	}
	

	glm::mat4 translate(glm::vec3 t)
	{
		return glm::mat4(
			1,    0,   0,  0,   //column 0
			0,    1,   0,  0,   //column 1
			0,    0,   1,  0,   //column 2
			t.x, t.y, t.z, 1    //column 3
		);
	}

	glm::mat4 getModelMatrix()
	{
		glm::mat4 rot = rotate(rotation);
		glm::mat4 tran = translate(translation);
		glm::mat4 scal = scale(size);

		return scal * rot * tran;
	}
}

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Transformations", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	Shader shader("shaders/vertexShader.vert", "shaders/fragmentShader.frag");

	MeshData cubeMeshData;
	createCube(1.0f, 1.0f, 1.0f, cubeMeshData);

	Mesh cubeMesh(&cubeMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(bgColor.r,bgColor.g,bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		//Draw
		shader.use();
		for (size_t i = 0; i < 3; i++)
		{
			shader.setMat4("_Model", transforms[i].getModelMatrix());
			cubeMesh.draw();
		}

		//Draw UI
		ImGui::Begin("Settings");
		ImGui::SliderFloat("Example slider", &exampleSliderFloat, 0.0f, 10.0f);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
}

void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}