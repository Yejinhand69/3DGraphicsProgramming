#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 model, view, projection;
uniform float time;

out vec2 TexCoord;
out vec3 Normal, FragWPos;

// Function to create a rotation matrix around the Z-axis
mat4 rotateZ(float angle) {
    float s = sin(angle);
    float c = cos(angle);
    return mat4(
        vec4( c, -s, 0.0, 0.0),
        vec4( s,  c, 0.0, 0.0),
        vec4( 0.0, 0.0, 1.0, 0.0),
        vec4( 0.0, 0.0, 0.0, 1.0)
    );
}

void main()
{
    // Define the pivot point of the object (where it should rotate around)
    vec3 pivot = vec3(20.0, 670.0, 0.0);  // Adjust as necessary

    // Translate the object to the origin, apply the rotation, then translate back
    vec4 localPos = vec4(aPos - pivot, 1.0);  // Translate to origin
    float rotationSpeed = 1.5;  // Rotation speed
    float angle = time * rotationSpeed;
    localPos = rotateZ(angle) * localPos;  // Apply rotation
    localPos.xyz += pivot;  // Translate back

    // Transform vertex position to world space
    vec4 posWS = model * localPos;

    // Pass transformed data to the fragment shader
    FragWPos = posWS.xyz;
    TexCoord = aTexCoord;
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // Calculate final vertex position
    gl_Position = projection * view * posWS;
}
