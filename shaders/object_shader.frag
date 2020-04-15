#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
  
uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;

void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Normalize the normal
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    // Specular lighting
    float specularStrength = 0.1f;
    vec3 viewDir = normalize( viewPos - FragPos );
    vec3 reflectDir = reflect( -lightDir, norm );
    float spec = pow(max(dot( viewDir, reflectDir ), 0.0), 16);
    vec3 specular = specularStrength * spec * lightColor;
  	
    // diffuse 
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
            
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
} 