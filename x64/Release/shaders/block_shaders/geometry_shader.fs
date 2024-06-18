#version 330 core
out vec4 FragColor;

uniform sampler2D textureA;

in vec2 TexCoord;

void main()
{
    FragColor = vec4(texture(textureA, TexCoord).rgb, 1.0);
}