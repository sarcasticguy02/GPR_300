#version 450                          
out vec4 FragColor;

in vec3 Normal;
in vec2 UV;

uniform sampler2D _WoodTexture;
uniform sampler2D _SteelTexture;
uniform float _Time;

void main(){         
    FragColor = vec4(UV.x, UV.y, (UV.x + UV.y)/2, 1.0f) * mix(texture(_SteelTexture, (UV * cos(_Time))), texture(_WoodTexture, UV), cos(_Time));
}
