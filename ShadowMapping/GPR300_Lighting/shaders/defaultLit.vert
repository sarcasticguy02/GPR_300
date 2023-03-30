#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUV;
layout (location = 3) in vec3 vTangent;

uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Projection;

out vec3 Normal;
out vec3 Tangent;
out vec2 UV;
out mat3 TBN;

out struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPos;
}v_out;


void main(){    
    v_out.WorldPos = vec3(_Model * vec4(vPos,1));

    Normal = vNormal;
    Tangent = normalize(vec3(_Model * vec4(vTangent, 0)));
    UV = vUV;
    vec3 Bitangent = cross(Normal, Tangent);
    //Calculate TBN
    Bitangent = normalize(vec3(_Model * vec4(Bitangent, 0)));
    Normal = normalize(vec3(_Model * vec4(Normal, 0)));
    TBN = mat3(
    Tangent.x, Bitangent.x, Normal.x,
    Tangent.y, Bitangent.y, Normal.y,
    Tangent.z, Bitangent.y, Normal.z
    );

    TBN = transpose(inverse(mat3(_Model))) * TBN;
    Normal = TBN * Normal;
    v_out.WorldNormal = Normal;
    gl_Position = _Projection * _View * _Model * vec4(vPos,1);
}
