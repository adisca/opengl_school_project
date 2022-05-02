#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
in float visibility;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
//shdaows
uniform sampler2D shadowMap;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
// other
uniform bool fogEnabled;
uniform float lightColorCoeff;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shadow;

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
}

void computeShadow() {
    // perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5 + 0.5;

    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;

    // Check whether current frag pos is in shadow
    float bias = 0.005f;
    shadow = (currentDepth - bias) > closestDepth ? 1.0f : 0.0f;

    if (currentDepth > 1.0f)
        shadow = 0.0f;
}

void main() 
{
    computeDirLight();

    computeShadow();

    //compute final vertex color
    vec3 color = min((ambient + (1.0f - shadow) * diffuse) * texture(diffuseTexture, fTexCoords).rgb + (1.0f - shadow) * specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
    if (fogEnabled) {
        color = vec3(mix(vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(color, 1.0f), visibility));
    }
    color = vec3(mix(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(color, 1.0f), lightColorCoeff));
    fColor = vec4(color, 1.0f);
}
