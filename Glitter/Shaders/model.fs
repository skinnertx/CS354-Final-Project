#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
  
uniform Material material;

void main()
{    
    FragColor = texture(texture_diffuse1, TexCoords) + vec4(material.diffuse, 0.0);
}