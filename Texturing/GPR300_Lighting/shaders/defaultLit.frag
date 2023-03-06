#version 450                          
out vec4 FragColor;

in vec3 Normal;
in vec2 UV;
in mat3 TBN;

uniform sampler2D _WoodTexture;
uniform sampler2D _SteelTexture;
uniform float _Time;

uniform sampler2D _NormalMap;

void main(){         
    vec3 normal = texture(_NormalMap, UV).rgb;
    FragColor = vec4(UV.x, UV.y, (UV.x + UV.y)/2, 1.0f) * mix(texture(_SteelTexture, (UV * cos(_Time))), texture(_WoodTexture, UV), cos(_Time));
}
