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

vec3 calcLight(Light light)
{
    //Ambient
    float coefficient = 1.0f;
	vec3 rgb = glm::vec3(100, 100, 100);
	vec3 ambient = rgb * coefficient;

    //Diffuse
    float dCoefficient = 1.0f;
    vec3 dir = normalize(vec3(100, 100, 100));
    vec3 normal = normalize(vec3(100, 100, 100));
    vec3 intensity = glm::vec3(100, 100, 100);
    vec3 diffuse = dCoefficient * dot(dir, normal) * intensity;

    //Specular (only thing that changes from phong to blinn-phong


    vec3 specular;
    return vec3(0);
}

void main(){
    vec3 normal = normalize(v_out.WorldNormal);
    FragColor = vec4(abs(normal),1.0f);
    //FragColor = vec4(v_out.WorldPos, 1);
}
