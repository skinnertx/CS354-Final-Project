#version 330 core
layout(location = 0) out vec4 normalColor;
layout(location = 1) out vec4 depthColor;

in vec3 normal;

float near = 0.1; 
float far  = 100.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{
    normalColor = vec4(normal, 1.0);
    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    depthColor = vec4(vec3(depth), 1.0);
}