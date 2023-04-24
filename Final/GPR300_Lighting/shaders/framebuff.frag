#version 450                          
out vec4 FragColor;

in vec2 uv;

uniform int post;
uniform int effect;
uniform float _Time;
uniform sampler2D text;

//Blur
//https://learnopengl.com/In-Practice/2D-Game/Postprocessing
/*uniform float res;
uniform float radius;
float offset = 1/300;
float offsets[9][2] = {
{-offset, offset},
{0, offset},
{offset, offset},
{-offset, 0},
{0, 0},
{offset, 0},
{-offset, -offset},
{0, -offset},
{offset, -offset}
};*/

const float kernel = 10.0;
const float weight = 1.0;

void main()
{
    bool up = true;
    vec4 color = texture(text, uv);
    vec4 finalColor;

    //inverse
    /*if (effect == 0)
    {
        FragColor = vec4(1 - color.x, 1 - color.y, 1 - color.z, 1);
    }
    //greyscale
    else if(effect == 1)
    {
        float avg = (color.x + color.y + color.z) / 3;
        color.x = avg;
        color.y = avg;
        color.z = avg;
        FragColor = color;
    }
    //bloodlust
    else if(effect == 2)
    {
        color.y = 0;
        color.z = 0;
        if(color.x == 0)
        {
            color.x = .2;
        }
            
        finalColor += color;
    }*/
    //blur
    //https://learnopengl.com/In-Practice/2D-Game/Postprocessing
    /*if(effect == 3)
    {
        vec3 newCol[9];
        for(int i = 0; i < 9; i++)
        {
            vec2 temp = vec2(offsets[i][0], offsets[i][1]);
            color += vec4(vec3(texture(text, uv * temp)), 0) * .1;
        }
        finalColor += color / 9;
    }*/

    //Horizontal Blur
    //https://www.shadertoy.com/view/llGSz3
    /*vec3 sum = vec3(0);
    float pixelSize = 1.0 / 1080;
    
    vec3 total = vec3(0);
    vec3 weightSum = vec3(0);
    for (float i = -kernel; i <= kernel; i++){
        total += texture(text, uv + vec2(i * pixelSize, 0.0)).xyz * weight;
        weightSum += weight;
    }
    
    sum = total / weightSum;
    
    FragColor = vec4(sum, 1.0);*/

    //Gaussian Blur
    

    //FragColor = color;
}