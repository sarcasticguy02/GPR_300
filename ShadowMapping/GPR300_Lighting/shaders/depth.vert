#version 450 
layout (location = 0) in vec3 vPos; 

uniform mat4 _View; //view projection
uniform mat4 _Model;

void main()
{
	gl_Position = _View * _Model * vec4(vPos, 1);
}