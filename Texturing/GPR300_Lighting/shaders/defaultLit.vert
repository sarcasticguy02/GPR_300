#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUV;
layout (location = 3) in vec3 vTangent;
layout (location = 4) in vec3 vBitagent;

uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Projection;

out vec3 Normal;
out vec2 UV;
out mat3 TBN;


void main(){    
    Normal = vNormal;
    UV = vUV;
    //Calculate TBN
    //TBN = 
    gl_Position = _Projection * _View * _Model * vec4(vPos,1);
}
