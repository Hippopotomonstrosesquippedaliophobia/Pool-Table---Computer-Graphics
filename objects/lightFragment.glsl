#version 330 core
out vec4 color;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;


uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
//uniform vec3 objectColor;
uniform vec3 lightType;

uniform sampler2D texture_diffuse1; //??


void main()
{
    // Ambient
    float ambientStrength = 0.8f;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.5f;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
   
    
    vec4 result;
    
    if (lightType.x == 0)
        result = vec4(0.0f);
    if (lightType.x == 1)
        result = vec4(ambient, 1.0f);
    if (lightType.x == 2)
        result = vec4(diffuse, 1.0f);
    if (lightType.x == 3)
        result = vec4(specular, 1.0f);
    if (lightType.x == 4)
        result = vec4((ambient + diffuse + specular), 1.0f);

    color = texture(texture_diffuse1, TexCoords) * result;

}
