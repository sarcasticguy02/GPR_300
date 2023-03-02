#version 450                          
out vec4 FragColor;

in vec3 Normal;
in vec2 UV;

void main(){         
    FragColor = vec4(UV.x, UV.y, 0.0, 1.0f);
}
