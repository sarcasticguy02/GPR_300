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
#include "EW/EwMath.h"
#include "EW/Camera.h"
#include "EW/Mesh.h"
#include "EW/Transform.h"
#include "EW/ShapeGen.h"

#include "iostream"

void processInput(GLFWwindow* window);
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mousePosCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void renderObjects(Shader& shader, ew::Transform& obj, ew::Mesh& mesh);

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
const float CAMERA_MOVE_SPEED = 5.0f;
const float CAMERA_ZOOM_SPEED = 3.0f;

Camera camera((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);

glm::vec3 bgColor = glm::vec3(0);
glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPosition = glm::vec3(0.0f, 3.0f, 0.0f);

bool wireFrame = false;

const char* woodFile = "wood.png";
const char* normalWood = "woodNormal.png";

float blurrSize;

GLuint createTexture(const char* filePath)
{
	//Generate a texture name
	GLuint texture;
	glGenTextures(1, &texture);
	//Bind out name to GL_TEXTURE_2D to make it a 2D texture
	glBindTexture(GL_TEXTURE_2D, texture);

	int width, height, numComponents;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* textureData = stbi_load(filePath, &width, &height, &numComponents, 0);

	switch (numComponents)
	{
	case 1:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R, width, height, 0, GL_R, GL_UNSIGNED_BYTE, textureData);
		break;
	case 2:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, textureData);
		break;
	case 3:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
		break;
	case 4:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
		break;
	default:
		break;
	}

	//Wrap horizontally
	glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//Wrap vertically
	glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//Magnifying
	glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//Minifying
	glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);

	return texture;
}

struct Material {
	glm::vec3 Color = glm::vec3(1);
	float AmbientK = .3f;
	float DiffuseK = .4f;
	float SpecularK = .3f;
	float Shininess = 1;
};

struct PLight {
	glm::vec3 color = glm::vec3(1);
	glm::vec3 pos = glm::vec3(0, 5, 0);
	float intensity = .8;
	float radius = 8;
};

PLight Plight;
Material mat;
//Effects
int currentBlur = 0;
const char* blurNames[4] =
{ "Horizontal", "Box", "Gaussian", "Directional"};
float kernel = 10, quality = 3, size = 8, angle = 45, strength = .05;
int samples = 50;

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Post Processing Stack Final", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetCursorPosCallback(window, mousePosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	//Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	//Used to draw shapes. This is the shader you will be completing.
	Shader litShader("shaders/defaultLit.vert", "shaders/defaultLit.frag");

	//Used to draw light sphere
	Shader unlitShader("shaders/defaultLit.vert", "shaders/unlit.frag");
	
	//Frame Buffer
	Shader framebuff("shaders/framebuff.vert", "shaders/framebuff.frag");

	GLuint wood = createTexture(woodFile);
	GLuint normalWoodFile = createTexture(normalWood);

	glActiveTexture(GL_TEXTURE0);
	//Bind out name to GL_TEXTURE_2D to make it a 2D texture
	glBindTexture(GL_TEXTURE_2D, normalWoodFile);

	glActiveTexture(GL_TEXTURE1);
	//Bind out name to GL_TEXTURE_2D to make it a 2D texture
	glBindTexture(GL_TEXTURE_2D, wood);

	ew::MeshData cubeMeshData;
	ew::createCube(1.0f, 1.0f, 1.0f, cubeMeshData);
	ew::MeshData sphereMeshData;
	ew::createSphere(0.5f, 64, sphereMeshData);
	ew::MeshData cylinderMeshData;
	ew::createCylinder(1.0f, 0.5f, 64, cylinderMeshData);
	ew::MeshData planeMeshData;
	ew::createPlane(1.0f, 1.0f, planeMeshData);

	ew::Mesh cubeMesh(&cubeMeshData);
	ew::Mesh sphereMesh(&sphereMeshData);
	ew::Mesh planeMesh(&planeMeshData);
	ew::Mesh cylinderMesh(&cylinderMeshData);

	ew::MeshData quadMeshData;
	ew::createQuad(2, 2, quadMeshData);
	ew::Mesh quadMesh(&quadMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Initialize shape transforms
	ew::Transform cubeTransform;
	ew::Transform sphereTransform;
	ew::Transform planeTransform;
	ew::Transform cylinderTransform;
	ew::Transform lightTransform;

	cubeTransform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
	sphereTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);

	planeTransform.position = glm::vec3(0.0f, -1.0f, 0.0f);
	planeTransform.scale = glm::vec3(10.0f);

	cylinderTransform.position = glm::vec3(2.0f, 0.0f, 0.0f);

	lightTransform.scale = glm::vec3(0.5f);

	//Create fbo
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	//Bind - we are drawing to this frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	//Texture Color Buffer
	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//Attaching color buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	//Create storage for depth components
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
	//Attach RBO to current FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	while (!glfwWindowShouldClose(window)) {
		lightTransform.position = Plight.pos;

		processInput(window);
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearColor(bgColor.r,bgColor.g,bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;
		
		litShader.setVec3("camPos", camera.getPosition());

		//Material
		litShader.setVec3("_Material.Color", glm::vec3(255, 255, 255));
		litShader.setFloat("_Material.AmbientK", mat.AmbientK);
		litShader.setFloat("_Material.DiffuseK", mat.DiffuseK);
		litShader.setFloat("_Material.SpecularK", mat.SpecularK);
		litShader.setFloat("_Material.Shininess", mat.Shininess);

		//Draw
		litShader.use();
		litShader.setMat4("_Projection", camera.getProjectionMatrix());
		litShader.setMat4("_View", camera.getViewMatrix());
		litShader.setFloat("time", time);
		litShader.setInt("shake", 0);
		//litShader.setVec3("_LightPos", lightTransform.position);

		litShader.setVec3("_PLights.color", Plight.color);
		litShader.setVec3("_PLights.pos", Plight.pos);
		litShader.setFloat("_PLights.intensity", Plight.intensity);
		litShader.setFloat("_PLights.radius", Plight.radius);

		litShader.setInt("_NormalMap", 0);

		litShader.setInt("_WoodTexture", 1);

		litShader.setFloat("_Time", time);

		renderObjects(litShader, cubeTransform, cubeMesh);
		renderObjects(litShader, sphereTransform, sphereMesh);
		renderObjects(litShader, cylinderTransform, cylinderMesh);
		renderObjects(litShader, planeTransform, planeMesh);

		//Draw light as a small sphere using unlit shader, ironically.
		unlitShader.use();
		unlitShader.setMat4("_Projection", camera.getProjectionMatrix());
		unlitShader.setMat4("_View", camera.getViewMatrix());
		unlitShader.setMat4("_Model", lightTransform.getModelMatrix());
		unlitShader.setVec3("_Color", lightColor);
		sphereMesh.draw();
		
		//Unbind (will reset to default frambuffer)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//Framebuffer
		framebuff.use();
		framebuff.setInt("text", 2);
		framebuff.setFloat("_Time", time);
		//Blur
		framebuff.setInt("activeBlur", currentBlur);
		framebuff.setFloat("kernel", kernel);
		framebuff.setFloat("quality", quality);
		framebuff.setFloat("size", size);
		framebuff.setInt("samples", samples);
		framebuff.setFloat("angle", angle);
		framebuff.setFloat("strength", strength);
		quadMesh.draw();

		//Draw UI
		ImGui::Begin("Material");
		ImGui::SliderFloat("Ambient", &mat.AmbientK, 0, 1);
		ImGui::SliderFloat("Diffuse", &mat.DiffuseK, 0, 1);
		ImGui::SliderFloat("Specular", &mat.SpecularK, 0, 1);
		ImGui::SliderFloat("Shininess", &mat.Shininess, 1, 512);
		ImGui::End();

		ImGui::Begin("Point Light");
		ImGui::ColorEdit3("Light Color", &Plight.color.r);
		ImGui::SliderFloat("Radius", &Plight.radius, 0, 10);
		ImGui::SliderFloat("Light Intentsity", &Plight.intensity, 0, 1);
		ImGui::SliderFloat3("Light Position", &Plight.pos.x, -5, 5);
		ImGui::End();

		ImGui::Begin("Blurring");
		ImGui::Combo("Blur", &currentBlur, blurNames, IM_ARRAYSIZE(blurNames));
		ImGui::SliderFloat("Kernel", &kernel, 1, 20);
		ImGui::SliderFloat("Quality", &quality, 1, 10);
		ImGui::SliderFloat("Size", &size, 1, 20);
		ImGui::SliderInt("Samples", &samples, 30, 70);
		ImGui::SliderFloat("Angle", &angle, 1, 90);
		ImGui::SliderFloat("Strength", &strength, 0, .1);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	//Delete
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &texture);

	glfwTerminate();
	return 0;
}

void renderObjects(Shader& shader, ew::Transform& obj, ew::Mesh& mesh)
{
	shader.setMat4("_Model", obj.getModelMatrix());
	mesh.draw();
}

//Author: Eric Winebrenner
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	camera.setAspectRatio((float)SCREEN_WIDTH / SCREEN_HEIGHT);
	glViewport(0, 0, width, height);
}
//Author: Eric Winebrenner
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	//Reset camera
	if (keycode == GLFW_KEY_R && action == GLFW_PRESS) {
		camera.setPosition(glm::vec3(0, 0, 5));
		camera.setYaw(-90.0f);
		camera.setPitch(0.0f);
		firstMouseInput = false;
	}
	if (keycode == GLFW_KEY_1 && action == GLFW_PRESS) {
		wireFrame = !wireFrame;
		glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
	}
}
//Author: Eric Winebrenner
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (abs(yoffset) > 0) {
		float fov = camera.getFov() - (float)yoffset * CAMERA_ZOOM_SPEED;
		camera.setFov(fov);
	}
}
//Author: Eric Winebrenner
void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
		return;
	}
	if (!firstMouseInput) {
		prevMouseX = xpos;
		prevMouseY = ypos;
		firstMouseInput = true;
	}
	float yaw = camera.getYaw() + (float)(xpos - prevMouseX) * MOUSE_SENSITIVITY;
	camera.setYaw(yaw);
	float pitch = camera.getPitch() - (float)(ypos - prevMouseY) * MOUSE_SENSITIVITY;
	pitch = glm::clamp(pitch, -89.9f, 89.9f);
	camera.setPitch(pitch);
	prevMouseX = xpos;
	prevMouseY = ypos;
}
//Author: Eric Winebrenner
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	//Toggle cursor lock
	if (button == MOUSE_TOGGLE_BUTTON && action == GLFW_PRESS) {
		int inputMode = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(window, GLFW_CURSOR, inputMode);
		glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
	}
}

//Author: Eric Winebrenner
//Returns -1, 0, or 1 depending on keys held
float getAxis(GLFWwindow* window, int positiveKey, int negativeKey) {
	float axis = 0.0f;
	if (glfwGetKey(window, positiveKey)) {
		axis++;
	}
	if (glfwGetKey(window, negativeKey)) {
		axis--;
	}
	return axis;
}

//Author: Eric Winebrenner
//Get input every frame
void processInput(GLFWwindow* window) {

	float moveAmnt = CAMERA_MOVE_SPEED * deltaTime;

	//Get camera vectors
	glm::vec3 forward = camera.getForward();
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
	glm::vec3 up = glm::normalize(glm::cross(forward, right));

	glm::vec3 position = camera.getPosition();
	position += forward * getAxis(window, GLFW_KEY_W, GLFW_KEY_S) * moveAmnt;
	position += right * getAxis(window, GLFW_KEY_D, GLFW_KEY_A) * moveAmnt;
	position += up * getAxis(window, GLFW_KEY_Q, GLFW_KEY_E) * moveAmnt;
	camera.setPosition(position);
}
