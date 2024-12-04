#version 330 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal, FragWPos;
in vec3 Tangent; // For the floor

uniform bool enableRainbowLight;
uniform bool enableDirectionalLight;

//_______________________________Textures______________________________//

// Uniforms for textures
//Floor
uniform sampler2D texture_floor_diffuse;
uniform sampler2D texture_floor_normal;

//House stand
uniform sampler2D texture_house;
uniform sampler2D texture_house2;

//House fan
uniform sampler2D texture_fan;
uniform sampler2D texture_fan2;

//Tree 
uniform sampler2D texture_tree_diffuse;
uniform sampler2D texture_tree_specular;

//Rocks
uniform sampler2D texture_rocks_diffuse;
uniform sampler2D texture_rocks_specular;
uniform sampler2D texture_rocks_normal;

//Horse
uniform sampler2D horse_texture_diffuse;
uniform sampler2D horse_texture_specular;

//_______________________________Textures______________________________//

// Light properties
uniform vec3 dirLightColour;
uniform vec3 dirLightDirection;
uniform vec3 cameraPosition;

uniform vec3 pointLightColourRainbow;
uniform vec3 pointLightPositionRainbow;
uniform float pointLightRangeRainbow;

//Light floor
uniform vec3 pointLightColour;
uniform vec3 pointLightPosition;
uniform float pointLightRange;

uniform float time;

uniform bool enableHDR;
uniform bool enableTonemap;
uniform bool enableExposure;
uniform bool enableContrast;
uniform bool enableSaturation;

uniform float exposure;
uniform float contrast;
uniform float saturation;

// Structure definition
struct Surface {
    vec3 worldPosition;
    vec3 diffuse;
    vec3 normal;
    float alpha;
    float specular;
    float shininess;
};

// Reusable functions
float square(float x) {
    return x * x;
}

float saturate(float x) {
    return clamp(x, 0.0, 1.0);
}

//---------------------------------Begin HDR---------------------------------//
//Tone mapping
vec3 filmicToneMap(vec3 color) {
    
    vec3 x = color * (1.0 + color / vec3(0.6));
    return x / (x + vec3(1.0));
    
}

vec3 acesToneMap(vec3 color) {
    
    vec3 A = color * (color + vec3(0.0245786));
    vec3 B = vec3(0.000090537) * (color * color) + vec3(0.000129768) * color;
    return (A - B) / (color + vec3(0.0001261));
    
}

vec3 combinedToneMap(vec3 color) {
    
    // Apply Filmic tone mapping first
    vec3 filmicColor = filmicToneMap(color);
    
    // Apply ACES tone mapping to the result of Filmic
    return acesToneMap(filmicColor);
}
//End of Tone Mapping

//Exposure
vec3 applyExposure(vec3 color, float exposure) {
    if(enableExposure)
    {
    return color * exposure;
    }
}

//Contrast
vec3 adjustContrast(vec3 color, float contrast) {
    if(enableContrast)
    {
    return ((color - 0.5) * contrast + 0.5);
    }
}

//Saturation
vec3 adjustSaturation(vec3 color, float saturation) {
    if(enableSaturation)
    {
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    return mix(vec3(gray), color, saturation);
    }
}
//---------------------------------End HDR---------------------------------//

vec3 animatePointLightPositionSquare(vec3 initialPosition, float sideLength, float speed) {
    float totalCycleTime = 4.0 * sideLength / speed;
    float timeInCycle = mod(time, totalCycleTime);

    if (timeInCycle < sideLength / speed) {
        return initialPosition + vec3(timeInCycle * speed, 0.0, 0.0);
    } else if (timeInCycle < 2.0 * sideLength / speed) {
        return initialPosition + vec3(sideLength, 0.0, (timeInCycle - sideLength / speed) * speed);
    } else if (timeInCycle < 3.0 * sideLength / speed) {
        return initialPosition + vec3(sideLength - (timeInCycle - 2.0 * sideLength / speed) * speed, 0.0, sideLength);
    } else {
        return initialPosition + vec3(0.0, 0.0, sideLength - (timeInCycle - 3.0 * sideLength / speed) * speed);
    }
}

vec3 rainbowColor(vec3 baseColor, float t) {
    float r = abs(sin(t + 0.0));
    float g = abs(sin(t + 2.0 * 3.14159 / 3.0));
    float b = abs(sin(t + 4.0 * 3.14159 / 3.0));
    return vec3(r, g, b);
}

Surface makeSurface(vec3 worldNormal, int surfaceType) {
    Surface surf;

    if (surfaceType == 0) 
    { // Floor
        vec4 sampledDiffuse = texture(texture_floor_diffuse, TexCoord);
        surf.worldPosition = FragWPos;
        surf.diffuse = sampledDiffuse.rgb;
        surf.normal = normalize(worldNormal);
        surf.alpha = sampledDiffuse.a;
        surf.specular = texture(texture_floor_diffuse, TexCoord).r;
        surf.shininess = 5.0;
    } 
    if (surfaceType == 1) 
    { // House stand
        vec4 sampledDiffuse = texture(texture_house, TexCoord);
        surf.worldPosition = FragWPos;
        surf.diffuse = sampledDiffuse.rgb;
        surf.normal = normalize(worldNormal); // Use normal directly for house
        surf.alpha = sampledDiffuse.a;
        surf.specular = texture(texture_house2, TexCoord).r;
        surf.shininess = 128.0;
    }
    if (surfaceType == 2) 
    { // House fan
        vec4 sampledDiffuse = texture(texture_fan, TexCoord);
        surf.worldPosition = FragWPos;
        surf.diffuse = sampledDiffuse.rgb;
        surf.normal = normalize(worldNormal); // Use normal directly for house
        surf.alpha = sampledDiffuse.a;
        surf.specular = texture(texture_fan2, TexCoord).r;
        surf.shininess = 128.0;
    }
    if (surfaceType == 3) 
    { // Tree 
        vec4 sampledDiffuse = texture(texture_tree_diffuse, TexCoord);
        surf.worldPosition = FragWPos;
        surf.diffuse = sampledDiffuse.rgb;
        surf.normal = normalize(worldNormal); // Use normal directly for tree
        surf.alpha = sampledDiffuse.a;
        surf.specular = texture(texture_tree_specular, TexCoord).r;
        surf.shininess = 0.15;
    }
    if (surfaceType == 4) 
    { // Rocks
        vec4 sampledDiffuse = texture(texture_rocks_diffuse, TexCoord);
        surf.worldPosition = FragWPos;
        surf.diffuse = sampledDiffuse.rgb;
        surf.normal = normalize(worldNormal); // Use normal directly for rocks
        surf.alpha = sampledDiffuse.a;
        surf.specular = texture(texture_rocks_specular, TexCoord).r;
        surf.shininess = 0.2;
    }
    if (surfaceType == 5) 
    { // Horse
        vec4 sampledDiffuse = texture(horse_texture_diffuse, TexCoord);
        surf.worldPosition = FragWPos;
        surf.diffuse = sampledDiffuse.rgb;
        surf.normal = normalize(worldNormal); // Use normal directly for rocks
        surf.alpha = sampledDiffuse.a;
        surf.specular = texture(horse_texture_specular, TexCoord).r;
        surf.shininess = 0.2;
    }

    return surf;
}

vec3 calcDirectionalLight(Surface surf) 
{
    if(enableDirectionalLight == true)
    {
    vec3 lightColour = dirLightColour;
    vec3 lightDirection = normalize(dirLightDirection);

    vec3 ambientContr = lightColour * 0.2;

    vec3 N = surf.normal;
    vec3 L = -lightDirection;
    float diffEq = max(0, dot(L, N));
    vec3 diffuseContr = lightColour * diffEq;

    vec3 V = normalize(cameraPosition - surf.worldPosition);
    vec3 H = normalize(V + L);
    float specBPEq = pow(max(0, dot(N, H)), surf.shininess * 2.0);
    vec3 specBPContr = lightColour * surf.specular * specBPEq;

    return ambientContr + diffuseContr + specBPContr;
    }
}

vec3 calcPointLight(Surface surf, vec3 pointLightPosition, vec3 pointLightColour, float pointLightRange) 
{
    if(enableRainbowLight)
    {
    vec3 lightDirection = normalize(FragWPos - pointLightPosition);

    vec3 ambientContr = pointLightColour * 0.2;

    vec3 N = surf.normal;
    vec3 L = -lightDirection;
    float diffEq = max(0, dot(L, N));
    vec3 diffuseContr = pointLightColour * diffEq * 0.1;

    vec3 V = normalize(cameraPosition - surf.worldPosition);
    vec3 H = normalize(V + L);
    float specBPEq = pow(max(0, dot(N, H)), surf.shininess * 2.0);
    vec3 specBPContr = pointLightColour * surf.specular * specBPEq;

    float d = distance(pointLightPosition, FragWPos);
    float dSqr = square(d);
    float invRSqr = pointLightRange;
    float d_iRSqrSqr = square(dSqr * invRSqr);
    float rangeAtt = square(max(0.0, 1.0 - d_iRSqrSqr));
    float att = rangeAtt;

    return (ambientContr + diffuseContr + specBPContr) * att;
    }
}

void main() {
    int surfaceType = 0; //index

    vec3 worldNormal;
    if (surfaceType == 0) 
    { // Floor
        vec3 sampledNormal = texture(texture_floor_normal, TexCoord).rgb;
        sampledNormal = sampledNormal * 2.0 - 1.0;

        vec3 T = normalize(Tangent);
        vec3 B = normalize(cross(Normal, T));
        vec3 N = normalize(Normal);
        mat3 TBN = mat3(T, B, N);

        worldNormal = normalize(TBN * sampledNormal);
    } 
    if (surfaceType == 1 || surfaceType == 2 || surfaceType == 3 || surfaceType == 4 || surfaceType == 5)  
    { //1 House //2 Fan //3 Tree //4 Rocks //5 horse
        worldNormal = normalize(Normal);
    }

    Surface surf = makeSurface(worldNormal, surfaceType);

    vec3 finalCol = (surf.diffuse * surf.shininess) * calcDirectionalLight(surf);

    if (surfaceType == 0) 
    { // Floor with animated light
        float sideLength = 11.0f;
        float speed = 5.0;
        vec3 animatedLightPosition = animatePointLightPositionSquare(vec3(-5.0, 2.0, -6.0), sideLength, speed);
        vec3 animatedLightColour = rainbowColor(pointLightColourRainbow, time);
        finalCol += calcPointLight(surf, animatedLightPosition, animatedLightColour, pointLightRangeRainbow);
    } 
    if (surfaceType == 1 || surfaceType == 2 || surfaceType == 3 || surfaceType == 4 || surfaceType == 5)  
    {//1 House //2 Fan //3 Tree //4 Rocks //5 horse
        finalCol += calcPointLight(surf, pointLightPosition, pointLightColour, pointLightRange);
    }

     // If HDR is enabled
    if (enableHDR) 
    {
        if(enableTonemap)
        {
        finalCol = combinedToneMap(finalCol);
        }
        if(enableExposure)
        {
        finalCol = applyExposure(finalCol, exposure);
        }
        if(enableContrast)
        {
        finalCol = adjustContrast(finalCol, contrast);
        }
        if(enableSaturation)
        {
        finalCol = adjustSaturation(finalCol, saturation);
        }
    }
    
    // If not
    FragColor = vec4(finalCol, surf.alpha);

    if(FragColor.a < 0.1)
    {discard;}

}
