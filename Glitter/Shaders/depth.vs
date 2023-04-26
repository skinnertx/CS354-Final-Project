#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform bool left;

out vec2 TexCoords;

void main()
{	
	if(left) {
		gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
	} else {
		gl_Position = vec4(aPos.x + 1.3f, aPos.y, 0.0, 1.0);
	}
	
	TexCoords = aTexCoords;
}