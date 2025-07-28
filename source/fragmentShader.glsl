#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Material properties from textures
uniform sampler2D diffuseMap;
uniform sampler2D specularMap; 
uniform sampler2D emissionMap; 

// Material properties from model
uniform float ka; 
uniform float kd; 
uniform float ks; 
uniform float Ns; 

// Global directional/positional light
uniform vec3 globalLightPos;
uniform vec3 globalLightColor;

// Point lights
#define NUM_POINT_LIGHTS 2
uniform vec3 pointLightPositions[NUM_POINT_LIGHTS];
uniform vec3 pointLightColors[NUM_POINT_LIGHTS];

// Camera position
uniform vec3 viewPos;

// --- Function to calculate lighting from a single point light ---
vec3 calculatePointLight(vec3 lightPos, vec3 lightColor, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 specularColor)
{
    // Ambient
    vec3 ambient = ka * lightColor;

    // Diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = kd * diff * lightColor;

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), Ns);
    vec3 specular = ks * spec * lightColor * specularColor;

    // Attenuation
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

    return (ambient + diffuse + specular) * attenuation;
}


void main()
{
    // --- Material Color from Textures ---
    // We no longer discard pixels based on the alpha channel.
    vec3 texColor = texture(diffuseMap, TexCoords).rgb;
    vec3 specularColor = texture(specularMap, TexCoords).rgb;
    vec3 emissionColor = texture(emissionMap, TexCoords).rgb;

    // --- Vector Calculations ---
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // --- Global Light Calculation ---
    vec3 globalLightDir = normalize(globalLightPos - FragPos); 
    
    vec3 ambient = ka * globalLightColor;
    vec3 diffuse = kd * max(dot(norm, globalLightDir), 0.0) * globalLightColor;
    
    vec3 halfwayDir = normalize(globalLightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), Ns);
    vec3 specular = ks * spec * globalLightColor * specularColor;

    vec3 totalLighting = ambient + diffuse + specular;

    // --- Add Contribution from Point Lights ---
    for(int i = 0; i < NUM_POINT_LIGHTS; i++)
    {
        totalLighting += calculatePointLight(pointLightPositions[i], pointLightColors[i], norm, FragPos, viewDir, specularColor);
    }
    
    // --- Final Color ---
    // The final alpha value is set to 1.0 to ensure the object is fully opaque.
    FragColor = vec4(totalLighting * texColor + emissionColor, 1.0);
}