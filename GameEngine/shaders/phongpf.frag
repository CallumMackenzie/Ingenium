#version 330 core

#define NR_POINT_LIGHTS 0

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

vec3 CalcDirLight(DirLight light, vec3 cnormal, vec3 viewDir, vec2 coordUV);  
vec3 CalcPointLight(PointLight light, vec3 cnormal, vec3 fragPos, vec3 viewDir, vec2 coordUV);  


layout (location = 0) out vec4 color;

uniform float u_time;
uniform Material material;
uniform DirLight dirLight;
#if NR_POINT_LIGHTS > 0
uniform PointLight pointLights[NR_POINT_LIGHTS];
#endif
uniform vec3 viewPos;

in vec3 UV;
in vec4 tint;
in vec3 normal;
in vec3 fragPos;

void main () 
{
    // vec2 cUV = vec2(UV.x / UV.z, 1 - (UV.y / UV.z));
    vec2 cUV = vec2(UV.x, 1 - UV.y);
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 result = CalcDirLight(dirLight, norm, viewDir, cUV);

#if NR_POINT_LIGHTS > 0
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, fragPos, viewDir, cUV);
#endif

    color = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 cnormal, vec3 viewDir, vec2 coordUV)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(cnormal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, cnormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, coordUV.xy).rgba + tint.rgba);
    vec3 diffuse  = light.diffuse  * diff * vec3(texture((material.diffuse), coordUV.xy).rgba + tint.rgba);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, coordUV.xy).rgba + tint.rgba);
    return (ambient + diffuse + specular);
} 
vec3 CalcPointLight(PointLight light, vec3 cnormal, vec3 cfragPos, vec3 viewDir, vec2 coordUV)
{
    vec3 lightDir = normalize(light.position - cfragPos);
    float diff = max(dot(cnormal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, cnormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float distance    = length(light.position - cfragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, coordUV.xy).rgba + tint.rgba);
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, coordUV.xy).rgba + tint.rgba);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, coordUV.xy).rgba + tint.rgba);
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}