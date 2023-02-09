#version 450                          
out vec4 FragColor;

in struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPos;
}v_out;

struct Light{
    vec3 pos;
    vec3 color;
    float intensity;
};


#define MAX_LIGHTS 8
uniform Light _Lights[MAX_LIGHTS];


void main(){
    vec3 normal = normalize(v_out.WorldNormal);
    FragColor = vec4(abs(normal),1.0f);
    //FragColor = vec4(WorldPos, 1);
}
