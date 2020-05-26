#version 330

in vec2 texCoord;
in float Ambient;

uniform sampler2D theTexture;

out vec4 colour;

void main()
{
	colour = texture(theTexture, texCoord) * Ambient;
}