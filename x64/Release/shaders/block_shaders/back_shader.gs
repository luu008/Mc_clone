#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 projection;
uniform mat4 view;

in mat4 model[];

out vec2 TexCoord;

void build_block(mat4 aModel)
{    
    TexCoord = vec2(1.0,1.0);
    gl_Position = projection * view * aModel * vec4(0.5f,-0.5f,-0.5f,1.0f); 
    EmitVertex();
    TexCoord = vec2(0.0,1.0);
    gl_Position = projection * view * aModel * vec4(-0.5f,-0.5f,-0.5f,1.0f); 
    EmitVertex();
    TexCoord = vec2(1.0,0.0);
    gl_Position = projection * view * aModel * vec4(0.5f,0.5f,-0.5f,1.0f); 
    EmitVertex();
    TexCoord = vec2(0.0,0.0);
    gl_Position = projection * view * aModel * vec4(-0.5f,0.5f,-0.5f,1.0f); 
    EmitVertex();
    EndPrimitive();
}

void main() {    
    build_block(model[0]);
}