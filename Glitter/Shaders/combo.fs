#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D normalTexture;
uniform sampler2D depthTexture;

void main()
{
    float nCol = texture(normalTexture, TexCoords.st).x;
    float dCol = texture(depthTexture, TexCoords.st).x;
    if(nCol > 0.0 || dCol > 0.0) {
        FragColor = vec4(1.0);
    } else {
        FragColor = vec4(0.0);
    }
    
}  