#version 450                          
out vec4 FragColor;

in vec3 Normal;
in vec2 UV;
in mat3 TBN;

uniform sampler2D _WoodTexture;
uniform sampler2D _SteelTexture;
uniform float _Time;

uniform sampler2D _NormalMap;

in struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPos;
}v_out;

struct Material{
    vec3 Color;
    float AmbientK;
    float DiffuseK;
    float SpecularK;
    float Shininess;
};

struct DLight{
    vec3 dir;
    vec3 color;
    float intensity;
};

uniform DLight _DLights;
uniform Material _Material;
uniform vec3 camPos;

//Shadows
in vec4 lightSpacePos;
uniform sampler2D _ShadowMap; 

vec3 calcBlinnPhong(DLight light, Material mat)
{
    //Ambient
    float intensity = light.intensity / 100;
	vec3 rgb = light.color;	
    vec3 ambient = rgb * mat.AmbientK * intensity;

    //Diffuse
    vec3 normal = normalize(v_out.WorldNormal);
    vec3 diffuse = mat.DiffuseK * max(dot(normalize(light.dir), normal), 0) * rgb * intensity;

    //Specular (only thing that changes from phong to blinn-phong)
    vec3 viewer = normalize(camPos - v_out.WorldPos);
    vec3 H = normalize(viewer + normalize(light.dir));
    vec3 specular = mat.SpecularK * pow(max(dot(normal, H), 0), mat.Shininess) * rgb * intensity;

    return ambient + diffuse + specular;
}

float calcShadow(sampler2D shadowMap, vec4 lightSpacePos){
    //Homogeneous Clip space to NDC coords [-w,w] to [-1,1]
    vec3 sampleCoord = lightSpacePos.xyz / lightSpacePos.w;
    //Convert from [-1,1] to [0,1] for sampling
    sampleCoord = sampleCoord * 0.5 + 0.5;
    float shadowMapDepth = texture(shadowMap, sampleCoord.xy).r;
    float myDepth = sampleCoord.z;
    //step(a,b) returns 1.0 if a >= b, 0.0 otherwise
    return step(shadowMapDepth,myDepth);
}



void main(){         
    vec3 normal = texture(_NormalMap, UV).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    vec3 totalLight = calcBlinnPhong(_DLights, _Material);
    float shadow = calcShadow(_ShadowMap, lightSpacePos); 
    //vec3 light = _Material.AmbientK + (_Material.DiffuseK + _Material.SpecularK) * (1.0 - shadow);

    FragColor = vec4(totalLight * _Material.Color, 1) * texture(_WoodTexture, UV);
    //FragColor = vec4(nWorld, 1.0f) * mix(texture(_SteelTexture, (UV * cos(_Time))), texture(_WoodTexture, UV), cos(_Time));
}