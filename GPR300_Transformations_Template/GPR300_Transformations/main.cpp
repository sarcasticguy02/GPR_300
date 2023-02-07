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

//UI Variables
float radius = 1.0f, speed = 1.0f, fov = 60.0f, height = 10.0f;
bool orthographic = false;

struct Transform
{
	glm::vec3 pos = glm::vec3(0, 0, 0), rot = glm::vec3(0, 0, 0), scale = glm::vec3(1, 1, 1);

	glm::mat4 size(glm::vec3 s)
	{
		return glm::mat4(
			s.x, 0, 0, 0,
			0, s.y, 0, 0,
			0, 0, s.z, 0,
			0, 0, 0, 1
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


	glm::mat4 position(glm::vec3 t)
	{
		return glm::mat4(
			1, 0, 0, t.x,   //column 0
			0, 1, 0, t.y,   //column 1
			0, 0, 1, t.z,   //column 2
			0, 0, 0, 1   //column 3
		);
	}

	glm::mat4 getModelMatrix()
	{
		return  position(pos) * size(scale) * rotate(rot);
	}
};

struct Camera
{
	glm::vec3 camPos, target = glm::vec3(0, 0, 0);
	glm::mat4 getViewMatrix()
	{
		glm::vec3 forward = glm::normalize(camPos - target);
		glm::vec3 u = glm::vec3(0, 1, 0);
		glm::vec3 right = glm::normalize(cross(forward, u));
		glm::vec3 up = glm::normalize(cross(right, forward));

		forward = -forward;

		glm::mat4 rCam = glm::mat4(
			right.x, right.y, right.z, 0,
			up.x, up.y, up.z, 0,
			forward.x, forward.y, forward.z, 0,
			0, 0, 0, 1
		);
		glm::mat4 tCam = glm::mat4(
			1, 0, 0, -camPos.x,
			0, 1, 0, -camPos.y,
			0, 0, 1, -camPos.z,
			0, 0, 0, 1
		);

		return rCam * tCam;
		//return glm::lookAt(camPos, camPos + forward, glm::vec3(0, 1, 0));
	};

	glm::mat4 getProjectionMatrix()
	{
		if (orthographic)
			return ortho(height, (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH, .01f, 100.0f);
		else
			return perpective(fov, (float)SCREEN_HEIGHT / (float)SCREEN_WIDTH, .01f, 100.0f);
	};

	glm::mat4 ortho(float h, float aspectRatio, float near, float far)
	{
		float r = h * aspectRatio;
		float l = -r;
		float t = h * 0.5f;
		float b = -t;
		
		return glm::mat4(
			2 / (r - l), 0, 0, -(r + l) / (r - l),
			0, 2 / (t - b), 0, -(t + b) / (t - b),
			0, 0, -2 / (far - near), -(far + near) / (far - near),
			0, 0, 0, 1
		);

	};
	glm::mat4 perpective(float fov, float aspectRatio, float near, float far)
	{
		float c = aspectRatio * tan(glm::radians(fov) / 2);
			return glm::mat4(
				1/(aspectRatio*c), 0, 0, 0,   //column 0
				0, 1/c, 0, 0,   //column 1
				0, 0, -((far+near)/(far-near)), -1,   //column 2
				0, 0, -((2 * far * near) / (far - near)), 1    //column 3
			);
	};
};

int main() {
	Transform trans;
	Camera cam;

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

	trans.scale = glm::vec3(1, 1, 1);
	trans.pos = glm::vec3(0, 0, 0);
	cam.camPos = glm::vec3(0, 0, 5);
	cam.target = glm::vec3(0, 0, 0);

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

		/*for (size_t i = 0; i < 3; i++)
		{
			shader.setMat4("_Model", Transform.getModelMatrix());
			cubeMesh.draw();
		}*/
		shader.setMat4("_Model", trans.getModelMatrix());
		shader.setMat4("_View", cam.getViewMatrix());
		shader.setMat4("_Projection", cam.getProjectionMatrix());
		cubeMesh.draw();

		//Draw UI
		ImGui::Begin("Settings");
		ImGui::SliderFloat("Radius", &radius, 0.0f, 10.0f);
		ImGui::SliderFloat("Speed", &speed, 0.0f, 10.0f);
		ImGui::SliderFloat("FOV", &fov, 0.0f, 100);
		ImGui::SliderFloat("Orthographic Height", &height, 0.0f, 10.0f);
		ImGui::Checkbox("Orthographic", &orthographic);
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