#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;
out vec2 pos;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    pos = (projection * vec4(vertex.xy, 0.0, 1.0)).xy;
    TexCoords = vertex.zw;
}