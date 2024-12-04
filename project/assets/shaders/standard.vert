#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aColor;
layout (location = 4) in vec3 aTangent;

uniform mat4 model, view, projection;

out vec2 TexCoord;

//new
out vec3 Normal, FragWPos;
out vec3 Tangent;

void main()
{
	TexCoord = aTexCoord;

	mat3 normalMatrix = mat3(transpose(inverse(model)));
	Normal = aNormal * normalMatrix;
	Tangent = normalize(normalMatrix * aTangent);

    vec4 pos_ws = model * vec4(aPos, 1.0f);

	FragWPos = pos_ws.xyz;
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}