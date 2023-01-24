#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <stdio.h>

//void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);

//TODO: Vertex shader source code
const char* vertexShaderSource =
"#version 450 \n"
"layout (location = 0) in vec3 vPos; \n"
"layout (location = 1) in vec3 vColor; \n"
"out vec3 Color; \n"
"uniform float _Time; \n"
"void main(){ \n"
"Color = vColor; \n"
"   gl_Position = vec4(abs(sin(_Time))*vPos, 1.0); \n"
"} \0";

//TODO: Fragment shader source code
const char* fragmentShaderSource =
"#version 450 \n"
"out vec4 FragColor; \n"
"in vec3 Color; \n"
"uniform float _Time; \n"
"void main(){ \n"
"   FragColor = vec4(abs(sin(_Time))*Color, 1.0); \n"
"} \0";

//TODO: Vertex data array
const float vertexData[] = {
	//x		y		z		color
	//Triangle 1
	-0.8,	-0.25,	0.0,	1.0, 0.0, 0.0, 1.0,
	-0.2,	-0.25,	0.0,	0.0, 1.0, 0.0, 1.0,
	-0.5,	0.25,	0.0,	0.0, 0.0, 1.0, 1.0,

	//Triangle 2
	-0.2,	-0.25,	0.0,	0.0, 0.0, 1.0, 1.0,
	0.4,	-0.25,	0.0,	1.0, 0.0, 0.0, 1.0,
	0.1,	0.25,	0.0,	0.0, 1.0, 0.0, 1.0,

	//Triangle 3
	0.4,	-0.25,	0.0,	0.0, 1.0, 0.0, 1.0,
	1,		-0.25,	0.0,	0.0, 0.0, 1.0, 1.0,
	0.7,	0.25,	0.0,	1.0, 0.0, 0.0, 1.0,
};

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGLExample", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);


	//TODO: Create and compile vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//TODO: Get vertex shader compilation status and output info log
	GLint vertexSuccess;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexSuccess);
	if (!vertexSuccess)
	{
		GLchar infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("Failed to compile vertex shader: &s");
	}
	
	//TODO: Create and compile fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//TODO: Get fragment shader compilation status and output info log
	GLint fragmentSuccess;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentSuccess);
	if (!fragmentSuccess)
	{
		GLchar infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf("Failed to compile fragment shader: &s");
	}

	//TODO: Create shader program
	GLuint shaderProgram = glCreateProgram();

	//TODO: Attach vertex and fragment shaders to shader program
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	//TODO: Link shader program
	glLinkProgram(shaderProgram);


	//TODO: Check for link status and output errors
	GLint shadeLink;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &shadeLink);
	if (!shadeLink)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("Failed to link shader program: %s", infoLog);
	}

	//TODO: Delete vertex + fragment shader objects
	//glDeleteShader(...)

	//TODO: Create and bind Vertex Array Object (VAO)
	//create a new vertex array object
	GLuint vertexArrayObject;
	glGenVertexArrays(1, &vertexArrayObject);
	//bind vertex array object
	glBindVertexArray(vertexArrayObject);

	//TODO: Create and bind Vertex Buffer Object (VBO), fill with vertexData
	//gernerate a buffer with an id
	GLuint vertexBufferObject;
	glGenBuffers(1, &vertexBufferObject);

	//bind buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);

	//create a new data store
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	//TODO: Define vertex attribute layout
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const void*)(sizeof(float)*3));
	glEnableVertexAttribArray(1);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.2f, 0.3f, 0.6f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//TODO:Use shader program
		glUseProgram(shaderProgram);

		//Set Uniform
		float time = (float)glfwGetTime();
		glUniform1f(glGetUniformLocation(shaderProgram, "_Time"), time);
		
		//TODO: Draw triangle (3 indices!)
		glDrawArrays(GL_TRIANGLES, 0, 9);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

