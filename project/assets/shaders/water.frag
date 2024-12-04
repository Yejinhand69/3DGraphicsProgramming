#version 330 core
layout (location = 0) out vec4 FragColor;

uniform sampler2D opaqueTex;
uniform sampler2D distortTex;

uniform vec2 resolution;
uniform float time;

in vec2 TexCoord;

void main()
{
	vec4 col = vec4(0.29, 0.58, 0.53, 0.5);

	vec2 distort = texture(distortTex, TexCoord + vec2(time * 0.5, 0.0)).xy;
	vec2 DistortPower =  distort.xy * 0.05;

	vec2 DistortedUV = TexCoord + DistortPower;

	vec4 screenCol = texture(opaqueTex, DistortedUV);

	FragColor = screenCol * col;
	
	if (FragColor.a < 0.1)
        discard;
}