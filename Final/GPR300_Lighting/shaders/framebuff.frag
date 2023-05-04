#version 450                          
out vec4 FragColor;

in vec2 uv;

uniform int post;
uniform int effect;
uniform float _Time;
uniform sampler2D text;

uniform int activeBlur;

uniform float kernel = 10.0;

//gaussian blur
uniform float quality = 3;
uniform float size = 8;

//directional blur
uniform int samples = 50;
uniform float angle = 45;
uniform float strength = .05;

void main()
{
    bool up = true;
    vec4 color = texture(text, uv);
    vec4 finalColor;

    float weight = 1.0;

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

    color.y = 0;
    color.z = 0;
    if(color.x == 0)
    {
        color.x = .2;
    }  
    FragColor += color;
    }*/

    //Horizontal Blur
    //https://www.shadertoy.com/view/llGSz3
    if(activeBlur == 0)
    {
        float horizontalSize = 1.0 / 1080;
        float verticleSize = 1.0/720;
    
        vec3 total = vec3(0);
        vec3 weightSum = vec3(0);
        for (float i = -kernel; i <= kernel; i++){
        total += texture(text, uv + vec2(i * horizontalSize, 0.0)).xyz * weight;
        weightSum += weight;
        }
    
        vec3 sum = total / weightSum;
    
        FragColor = vec4(sum, 1.0);

    }
    //Box Blur
    //https://www.shadertoy.com/view/llGSz3
    else if(activeBlur == 1)
    {
        float horizontalSize = 1.0 / 1080;
        float verticleSize = 1.0/720;
    
        vec3 total = vec3(0);
        vec3 weightSum = vec3(0);
        for (float i = -kernel; i <= kernel; i++){
            total += texture(text, uv + vec2(i * horizontalSize, 0.0)).xyz * weight;
            total += texture(text, uv + vec2(0.0, i * verticleSize)).xyz * weight;
            weightSum += weight * 2;
        }
    
        vec3 sum = total / weightSum;
    
        FragColor = vec4(sum, 1.0);
    }
    //Gaussian Blur
    //https://www.shadertoy.com/view/Xltfzj
    else if(activeBlur == 2)
    {
        float tau = 6.2831;
        float dir = 16;

        vec2 radius = size / vec2(1080, 720);

        vec4 total = vec4(0);
        vec4 weightSum = vec4(0);

        for(float i = 0.0; i < tau; i += tau/dir)
        {
            for(float j = 1.0/quality; j <= 1.0; j += 1.0/quality)
            {
                total += texture(text, uv + vec2(cos(i), sin(i)) * radius * j) * weight;
                weightSum += weight;
            }
        }

        total /= (weightSum / (quality / (dir - 15))) * 2;
        FragColor = total;
    }
    //Directional Blur
    //https://www.shadertoy.com/view/NltSD4
    else if(activeBlur == 3)
    {
        float r = radians(angle);
        vec2 dir = vec2(sin(r), cos(r));

        vec2 angles = strength * dir;
        vec3 total = vec3(0);
        vec3 weightSum = vec3(0);
        float delta = 2/float(samples);
        for(float i = -1.0; i < 1; i += delta)
        {
            total += (texture(text, uv - vec2(angles.x * i, angles.y * i)).rgb * delta * .5) * weight;
            weightSum += weight * delta / 2;
        }
        total /= weightSum;

        FragColor = vec4(total, 1);
    }

    //FragColor = color;
}