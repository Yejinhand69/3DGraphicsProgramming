#version 330 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal, FragWPos;

uniform sampler2D texture_lantern;
uniform sampler2D texture_roadllamp;
uniform sampler2D texture_lantern_emissive;

uniform vec3 pointLightColour1;
uniform vec3 pointLightPosition1;
uniform float pointLightRange1;

uniform vec3 pointLightColour2;
uniform vec3 pointLightPosition2;
uniform float pointLightRange2;

uniform vec3 pointLightColour3;
uniform vec3 pointLightPosition3;
uniform float pointLightRange3;

uniform vec3 cameraPosition;

uniform bool enableLanternLeft;
uniform bool enableLanternMid;
uniform bool enableLanternRight;

//Structure//
struct Surface
{
    vec3 worldPosition;
    vec3 diffuse;
    vec3 normal;
    float alpha;
    float specular;
    float shininess;
    vec3 emissive;
};

Surface makeSurface()
{
    Surface surf;

    vec4 sampledDiffuse = texture(texture_lantern, TexCoord);
    vec4 sampledEmissive = texture(texture_lantern_emissive, TexCoord);
    surf.worldPosition = FragWPos;
    surf.diffuse = sampledDiffuse.rgb;
    surf.normal = Normal;
    surf.alpha = sampledDiffuse.a;
    surf.specular = texture(texture_roadllamp,TexCoord).r;
    surf.shininess = 1.0;
    surf.emissive = sampledEmissive.rgb;

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

vec3 calcPointLight01(Surface surf)
{
    if(enableLanternLeft == true)
    {
    vec3 lightColour = pointLightColour1 ;
    vec3 lightWorldPosition = pointLightPosition1;

    vec3 lightDirection = normalize(FragWPos - lightWorldPosition);

    //Ambient
    vec3 ambientContr = lightColour * 0.2;

    //Diffuse
    vec3 N = surf.normal;
    vec3 L = -lightDirection;
    float diffEq = max(0,dot(L,N));
    vec3 diffuseContr = lightColour * diffEq;

    //Specular(Blinn-Phong)//Assignment can choose between phong or blinn-phong//End//
    vec3 V = normalize(cameraPosition - surf.worldPosition);
    vec3 H = normalize(V + L);
    float specBPEq = pow(max(0,dot(N,H)), surf.shininess * 2.0);
    vec3 specBPContr = lightColour * surf.specular *  specBPEq;

    //Attenuation (Unity RP) //Uses lightRange
    float d = distance(lightWorldPosition, FragWPos);
    float dSqr = square(d);
    float invRSqr = pointLightRange1;
    float d_iRSqrSqr = square(dSqr * invRSqr);
    float rangeAtt = square(max(0.0, 1.0 - d_iRSqrSqr));
    float att = rangeAtt;

    vec3 lightContribution = ambientContr + diffuseContr + specBPContr;
    return lightContribution * att;
    }
}

vec3 calcPointLight02(Surface surf)
{
    if(enableLanternRight == true)
    {
    vec3 lightColour = pointLightColour2 ;
    vec3 lightWorldPosition = pointLightPosition2;

    vec3 lightDirection = normalize(FragWPos - lightWorldPosition);

    //Ambient
    vec3 ambientContr = lightColour * 0.2;

    //Diffuse
    vec3 N = surf.normal;
    vec3 L = -lightDirection;
    float diffEq = max(0,dot(L,N));
    vec3 diffuseContr = lightColour * diffEq;

    //Specular(Blinn-Phong)//Assignment can choose between phong or blinn-phong//End//
    vec3 V = normalize(cameraPosition - surf.worldPosition);
    vec3 H = normalize(V + L);
    float specBPEq = pow(max(0,dot(N,H)), surf.shininess * 2.0);
    vec3 specBPContr = lightColour * surf.specular *  specBPEq;

    //Attenuation (Unity RP) //Uses lightRange
    float d = distance(lightWorldPosition, FragWPos);
    float dSqr = square(d);
    float invRSqr = pointLightRange2;
    float d_iRSqrSqr = square(dSqr * invRSqr);
    float rangeAtt = square(max(0.0, 1.0 - d_iRSqrSqr));
    float att = rangeAtt;

    vec3 lightContribution = ambientContr + diffuseContr + specBPContr;
    return lightContribution * att;
    }
}

vec3 calcPointLight03(Surface surf)
{
    if(enableLanternMid == true)
    {
    vec3 lightColour = pointLightColour3 ;
    vec3 lightWorldPosition = pointLightPosition3;

    vec3 lightDirection = normalize(FragWPos - lightWorldPosition);

    //Ambient
    vec3 ambientContr = lightColour * 0.2;

    //Diffuse
    vec3 N = surf.normal;
    vec3 L = -lightDirection;
    float diffEq = max(0,dot(L,N));
    vec3 diffuseContr = lightColour * diffEq;

    //Specular(Blinn-Phong)//Assignment can choose between phong or blinn-phong//End//
    vec3 V = normalize(cameraPosition - surf.worldPosition);
    vec3 H = normalize(V + L);
    float specBPEq = pow(max(0,dot(N,H)), surf.shininess * 2.0);
    vec3 specBPContr = lightColour * surf.specular *  specBPEq;

    //Attenuation (Unity RP) //Uses lightRange
    float d = distance(lightWorldPosition, FragWPos);
    float dSqr = square(d);
    float invRSqr = pointLightRange3;
    float d_iRSqrSqr = square(dSqr * invRSqr);
    float rangeAtt = square(max(0.0, 1.0 - d_iRSqrSqr));
    float att = rangeAtt;

    vec3 lightContribution = ambientContr + diffuseContr + specBPContr;
    return lightContribution * att;
    }
}

void main()
{
    //FragColor = texture(texture_lantern,TexCoord);

    Surface surf = makeSurface();

    vec3 baseColor = surf.diffuse * 0.5; // Small ambient term for visibility

    
    vec3 finalCol =  (baseColor + surf.diffuse + surf.emissive) * calcPointLight01(surf) + calcPointLight02(surf) + calcPointLight03(surf);
    
    

    FragColor = vec4(finalCol, surf.alpha);
}