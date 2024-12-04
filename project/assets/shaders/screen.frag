#version 330 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D mainTex;
uniform vec2 cursor, resolution;
uniform float time;

uniform bool enableSepia;
uniform bool enableFilmGrain;
uniform bool enableBadTVSignal;
uniform bool enableVignette;

uniform float filmGrainAmount;
uniform float tvEffectStrength;
uniform float vignettePower;

float vignette(float power)
{
    vec2 uv = TexCoord;

    uv *=  1.0 - uv.yx;  

    float vig = uv.x * uv.y * 10.0; 

    vig = pow(vig, power);

    return vig;
}

//Sepia film grain post processing
vec3 applySepia(vec3 color)
{
    vec3 sepia;
    sepia.r = dot(color, vec3(0.393, 0.769, 0.189));
    sepia.g = dot(color, vec3(0.349, 0.686, 0.168));
    sepia.b = dot(color, vec3(0.272, 0.534, 0.131));
    return sepia;
}

float random(vec2 uv)
{
    return fract(sin(dot(uv.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 applyFilmGrain(vec3 color, vec2 uv)
{
    float noise = random(uv + time) * filmGrainAmount;
    return color + vec3(noise);
}

vec3 applyBadTVSignal(vec3 color, vec2 uv, float strength)
{
    // Base film grain effect
    float noise = random(uv + time) * strength;
    color += vec3(noise);

    // Add horizontal banding effect
    float bandFrequency = 5.0f;
    float bandSpeed = 2.0;      // Controls the speed of the bands
    float bandThickness = 0.1; // Controls the thickness of the bands

    // Randomize band position over time
    float randomOffset = random(vec2(time, uv.y)) * 2.0 - 1.0; // Random offset between -1 and 1
    float bandPosition = mod(uv.y * bandFrequency + time * bandSpeed + randomOffset, 1.0);

    if (bandPosition < bandThickness)
    {
        color *= vec3(1.0 - strength); // Darken color in the band
    }

    // Add random "empty" horizontal lines
    float emptyLineFrequency = 50.0;
    float emptyLineSpeed = 1.5;
    float emptyLineThickness = 0.005;

    float emptyLinePosition = mod(uv.y * emptyLineFrequency + time * emptyLineSpeed, 1.0);
    if (emptyLinePosition < emptyLineThickness)
    {
          color = mix(color, vec3(0.0), strength);
    }

    return color;
}


void main()
{   
    vec3 col = texture(mainTex, TexCoord).rgb;

    if(enableSepia)
    {
        col = applySepia(col);
    }

    if(enableFilmGrain)
    {
        col = applyFilmGrain(col, TexCoord);
    }

    if(enableBadTVSignal)
    {
        col = applyBadTVSignal(col, TexCoord, tvEffectStrength);
    }

    if(enableVignette)
    {
        float vig = vignette(vignettePower);
        float ivig = 1.0 - vig;
        col = vec3(0.0, 0.0, 0.0) * ivig + col * (1-ivig);
    }
    
    FragColor = vec4(col, 1.0);
}