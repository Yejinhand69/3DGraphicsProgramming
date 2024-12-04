#version 330 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal, FragWPos;

uniform sampler2D texture_roadllamp;
uniform sampler2D texture_specular;

uniform vec3 pointLightColour0;
uniform vec3 pointLightPosition0;
uniform float pointLightRange0;
uniform vec3 cameraPosition;

uniform bool enableRoadlamp;

//Structure//
struct Surface
{
    vec3 worldPosition;
    vec3 diffuse;
    vec3 normal;
    float alpha;
    float specular;
    float shininess;
};

Surface makeSurface()
{
    Surface surf;

    vec4 sampledDiffuse = texture(texture_roadllamp, TexCoord);
    surf.worldPosition = FragWPos;
    surf.diffuse = sampledDiffuse.rgb;
    surf.normal = Normal;
    surf.alpha = sampledDiffuse.a;
    surf.specular = texture(texture_specular,TexCoord).r;
    surf.shininess = 128.0;

    return surf;
}
//End Structure & Reusable Function//
//Reusable functions//
float square(float x)
{
    return x * x;
}

float saturate(float x)
{
    return clamp(x, 0.0, 1.0);
}
//Reusable functions//

vec3 calcPointLight(Surface surf)
{
    if(enableRoadlamp == true)
    {
    vec3 lightColour = pointLightColour0 ;
    vec3 lightWorldPosition = pointLightPosition0;

    //vec3 lightDirection = normalize(FragWPos - lightWorldPosition);
    vec3 lightDirection = normalize(lightWorldPosition - surf.worldPosition);

    //Ambient
    vec3 ambientContr = lightColour * 0.2;

    //Diffuse
    vec3 N = surf.normal;
    vec3 L = -lightDirection;
    float diffEq = max(0,dot(L,N));
    vec3 diffuseContr = lightColour * diffEq;

    vec3 V = normalize(cameraPosition - surf.worldPosition);
    vec3 H = normalize(V + L);
    float specBPEq = pow(max(0,dot(N,H)), surf.shininess * 2.0);
    vec3 specBPContr = lightColour * surf.specular *  specBPEq;

    //Attenuation (Unity RP) //Uses lightRange
    float d = distance(lightWorldPosition, FragWPos);
    float dSqr = square(d);
    float invRSqr = pointLightRange0;
    float d_iRSqrSqr = square(dSqr * invRSqr);
    float rangeAtt = square(max(0.0, 1.0 - d_iRSqrSqr));
    float att = rangeAtt;

    vec3 lightContribution = ambientContr + diffuseContr + specBPContr;
    return lightContribution * att;
    }
}

void main()
{
    //FragColor = texture(texture_roadllamp,TexCoord);

    Surface surf = makeSurface();

    vec3 baseColor = surf.diffuse * 0.8; // Small ambient term for visibility
    
    vec3 finalCol = baseColor + surf.diffuse * calcPointLight(surf);

    FragColor = vec4(finalCol, surf.alpha);
  
}