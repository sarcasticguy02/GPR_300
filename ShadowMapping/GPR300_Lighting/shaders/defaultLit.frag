#version 450                          
out vec4 FragColor;

in vec3 Normal;
in vec2 UV;
in mat3 TBN;
in vec4 lightSpacePos;

uniform sampler2D _WoodTexture;
uniform sampler2D _SteelTexture;
uniform float _Time;
uniform sampler2D _ShadowMap;

uniform sampler2D _NormalMap;
uniform float _NormalIntensity;

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
    vec3 pos;
    vec3 dir;
    vec3 color;
    float intensity;
};

uniform DLight _DLights;
uniform Material _Material;
uniform vec3 camPos;

float calcShadow(sampler2D map, vec4 lightSpace)
{
    //Homogeneous Clip space to NDC coords [-w, w] to [-1, 1]
    vec3 sampleCoord = lightSpace.xyz  /lightSpace.w;
    //Convert from [-1, 1] to [0, 1] for sampling
    sampleCoord = sampleCoord * 0.5 + 0.5;
    float shadowMapDepth = texture(map, sampleCoord.xy).r;
    float myDepth = sampleCoord.z;
    //step(a, b) returns 1 if a >= b, 0 otherwise
    return step(shadowMapDepth, myDepth);
}

void main(){         
    vec3 normal = texture(_NormalMap, UV).rgb;
    normal = normalize(normal * 2.0 - 1.0) * vec3(_NormalIntensity, _NormalIntensity, 1);

    //Ambient
    float intensity = _DLights.intensity / 100;
	vec3 rgb = _DLights.color;	
    vec3 ambient = rgb * _Material.AmbientK * intensity;

    //Diffuse
    vec3 Dnormal = normalize(v_out.WorldNormal);
    vec3 diffuse = _Material.DiffuseK * max(dot(normalize(_DLights.dir), Dnormal), 0) * rgb * intensity;

    //Specular (only thing that changes from phong to blinn-phong)
    vec3 viewer = normalize(camPos - v_out.WorldPos);
    vec3 H = normalize(viewer + normalize(_DLights.dir));
    vec3 specular = _Material.SpecularK * pow(max(dot(Dnormal, H), 0), _Material.Shininess) * rgb * intensity;

    vec3 totalLight = ambient + diffuse + specular;

    float shadow = calcShadow(_ShadowMap, lightSpacePos);
    vec3 light = ambient + (diffuse + specular) * (1.0 - shadow);

    FragColor = vec4(totalLight * _Material.Color, 1) * texture(_WoodTexture, UV);
    //FragColor = vec4(nWorld, 1.0f) * mix(texture(_SteelTexture, (UV * cos(_Time))), texture(_WoodTexture, UV), cos(_Time));
}