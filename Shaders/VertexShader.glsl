#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in float ambient;

uniform mat4 mvp;

out vec2 texCoord;
out float Ambient;

void main()
{
	gl_Position = mvp * vec4(pos, 1.0f);
	texCoord = tex;
	Ambient = ambient;
}