#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in float ambient;

uniform mat4 model;
uniform mat4 viewProjection;

out vec2 texCoord;
out float Ambient;

void main()
{
	gl_Position = viewProjection * model * vec4(pos, 1.0f);
	texCoord = vec2(tex.x / 2, tex.y / 2);
	Ambient = ambient / 10;
}