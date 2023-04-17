#version 450                          
out vec4 FragColor;

in vec2 uv;

uniform int post;
uniform int effect;
uniform float _Time;
uniform sampler2D text;

void main()
{
    bool up = true;
    vec4 color = texture(text, uv);
    if(post == 0)
    {
        FragColor = color;
    }
    else
    {
        //inverse
        if (effect == 0)
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
            //https://webglfundamentals.org/webgl/lessons/webgl-qna-how-to-process-particle-positions.html
            FragColor = color;
        }
    }
}