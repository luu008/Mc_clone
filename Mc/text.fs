#version 330 core
in vec2 TexCoords;
in vec2 pos;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;
uniform bool type;
uniform int size;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    if (type)
    {
        float alpha = 0;
        int num = 0;
        for(float i = -0.5; i <= 0.5; i += 0.05) 
        {
            for(float j = -0.5; j <= 0.5; j += 0.05) 
            {
                if (i == 0 && j == 0)
                    alpha += texture(text, vec2((int(TexCoords.x * size) + i) / float(size), (int(TexCoords.y * size) + j) / float(size))).r * 10;
                else
                    alpha += texture(text, vec2((int(TexCoords.x * size) + i) / float(size), (int(TexCoords.y * size) + j) / float(size))).r;
                num++;
            }
        }
        alpha /= num;
        if (!(alpha <= 0.5))
            alpha = 1;
        sampled = vec4(1.0, 1.0, 1.0, alpha);
    }
    color = vec4(textColor, 1.0) * sampled;
}