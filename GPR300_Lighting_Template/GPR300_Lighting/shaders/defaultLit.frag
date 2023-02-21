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

struct DLight{
    vec3 dir;
    vec3 color;
    float intensity;
};

struct Material{
    vec3 Color;
    float AmbientK;
    float DiffuseK;
    float SpecularK;
    float Shininess;
};


#define MAX_LIGHTS 8
uniform Light _Lights[MAX_LIGHTS];
uniform DLight _DLights;
uniform Material _Material;
uniform vec3 camPos;
uniform vec3 _Color;

vec3 calcPhong(Light light, Material mat)
{
    //Ambient
	vec3 rgb = light.color;
	vec3 ambient = rgb * mat.AmbientK;

    //Diffuse
    vec3 dir = normalize(light.pos - v_out.WorldPos);
    vec3 normal = normalize(v_out.WorldNormal);
    vec3 diffuse = mat.DiffuseK * max(dot(dir, normal), 0) * light.color * light.intensity;

    //Specular (only thing that changes from phong to blinn-phong)
    vec3 viewer = normalize(camPos - v_out.WorldPos);
    vec3 reflected = normalize(reflect(-dir, normal));
    vec3 specular = mat.SpecularK * pow(max(dot(reflected, viewer), 0), mat.Shininess) * rgb * light.intensity;

    return ambient + diffuse + specular;
}

vec3 calcBlinnPhong(DLight light, Material mat)
{
    //Ambient
    float intensity = light.intensity / 100;
	vec3 rgb = light.color;	vec3 ambient = rgb * mat.AmbientK * intensity;

    //Diffuse
    //vec3 dir = normalize(light.pos - v_out.WorldNormal);
    vec3 normal = normalize(v_out.WorldNormal);
    vec3 diffuse = mat.DiffuseK * max(dot(normalize(light.dir), normal), 0) * rgb * intensity;

    //Specular (only thing that changes from phong to blinn-phong)
    vec3 viewer = normalize(camPos - v_out.WorldPos);
    vec3 H = normalize(viewer + normalize(light.dir));
    vec3 specular = mat.SpecularK * pow(max(dot(normal, H), 0), mat.Shininess) * rgb * intensity;

    return ambient + diffuse + specular;
    //return ambient;
}

void main(){
    vec3 normal = normalize(v_out.WorldNormal);
    vec3 BlinnPhong = calcBlinnPhong(_DLights, _Material);
    FragColor = vec4(BlinnPhong * _Material.Color, 1);
    //FragColor = vec4(v_out.WorldPos, 1);
}
