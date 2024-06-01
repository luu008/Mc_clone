#ifndef TEXT_H
#define TEXT_H
#include <glad/glad.h>

#include <iostream>
#include <map>
#include <string>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <ft2build.h>
class text
{
public:
    const char* ID;

    /// Holds all state information relevant to a character as loaded using FreeType
    struct Character {
        GLuint TextureID = -1;   // ID handle of the glyph texture
        glm::ivec2 Size = glm::ivec2(0, 0);    // Size of glyph
        glm::ivec2 Bearing = glm::ivec2(0, 0);   // Offset from baseline to left/top of glyph
        GLuint Advance = -1;    // Horizontal offset to advance to next glyph
    };

    std::map<wchar_t, Character> Characters;
	text(const char* trueType, unsigned int width, unsigned int height);
    int loadText(GLFWwindow* window);
    int RenderText(Shader& shader, std::wstring text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, bool type, int size);
    int RenderText(Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, bool type, int size);
    void setUsingSize(unsigned int width, unsigned int height);
    int deleteVaoAndVbo();
    int getError();
    std::string errorCodeToName(int errorCode);

private:
    unsigned int textVBO, textVAO;
    unsigned int WIN_WIDTH = 800, WIN_HEIGHT = 600, SCR_WIDTH, SCR_HEIGHT;
    std::wstring errorList = L"";
    bool loadType = false;
    std::string list[2] = {
        "Not init the characters.",
        "Not load then delete."
    };
};

text::text(const char* trueType, unsigned int width, unsigned int height)
{
    ID = trueType;
    SCR_HEIGHT = height;
    SCR_WIDTH = width;
}
int text::loadText(GLFWwindow* window)
{
    // FreeType
    FT_Library ft;

    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "------------------------------------" << std::endl << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }
    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, ID, 0, &face))
    {
        std::cout << "------------------------------------" << std::endl << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    FT_Select_Charmap(face, ft_encoding_unicode);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //禁用字节对齐限制
    
    for (wchar_t c = 0; c < 65535 && !glfwWindowShouldClose(window); c++)
    {
        if (c % 500 == 0)
        {
            glClearColor(c / 65535.0f, c / 65535.0f, c / 65535.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        // 加载字符的字形 

        if (FT_Load_Glyph(face, FT_Get_Char_Index(face, c), FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // 生成纹理
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // 设置纹理选项
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 储存字符供之后使用
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<wchar_t, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 2);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    loadType = true;
    return 0;
}
int text::RenderText(Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, bool type, int size)
{
    if (!loadType)
    {
        errorList.push_back(1);
        return -1;
    }
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Iterate through all characters
    // Activate corresponding render state	
    shader.use();
    shader.setVec3("textColor", color);
    glActiveTexture(GL_TEXTURE2);
    glBindVertexArray(textVAO);


    for (int c = 0; c < text.size(); c++)
    {
        Character ch = Characters[text[c]];//*c

        GLfloat xpos = x * ((float)WIN_WIDTH / (float)SCR_WIDTH) + ch.Bearing.x * scale;
        GLfloat ypos = y * ((float)WIN_HEIGHT / (float)SCR_HEIGHT) - (ch.Size.y - ch.Bearing.y) * scale;
        GLfloat w = ch.Size.x * scale * (glm::min)((float)WIN_HEIGHT / (float)SCR_HEIGHT, (float)WIN_WIDTH / (float)SCR_WIDTH);
        GLfloat h = ch.Size.y * scale * glm::min((float)WIN_HEIGHT / (float)SCR_HEIGHT, (float)WIN_WIDTH / (float)SCR_WIDTH);
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }
        };
        // Render glyph texture over quad
        glBindVertexArray(textVAO);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, vertices, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Activate corresponding render state	
        glm::mat4 projection = glm::ortho(0.0f, (float)WIN_WIDTH, 0.0f, (float)WIN_HEIGHT);
        shader.use();
        shader.setVec3("textColor", color);
        shader.setInt("text", 2);
        shader.setMat4("projection", projection);
        shader.setBool("type", type);
        shader.setInt("size", size);

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glActiveTexture(GL_TEXTURE2);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    return 0;
}
//这个是wstring
int text::RenderText(Shader& shader, std::wstring text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, bool type, int size)
{
    if (!loadType)
    {
        errorList.push_back(1);
        return -1;
    }
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Iterate through all characters
    // Activate corresponding render state	
    shader.use();
    shader.setVec3("textColor", color);
    glActiveTexture(GL_TEXTURE2);
    glBindVertexArray(textVAO);


    for (int c = 0; c < text.size(); c++)
    {
        
        Character ch = Characters[text[c]];//*c

        GLfloat xpos = x * ((float)WIN_WIDTH / (float)SCR_WIDTH) + ch.Bearing.x * scale;
        GLfloat ypos = y * ((float)WIN_HEIGHT / (float)SCR_HEIGHT) - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale * glm::min((float)WIN_HEIGHT / (float)SCR_HEIGHT, (float)WIN_WIDTH / (float)SCR_WIDTH);
        GLfloat h = ch.Size.y * scale * glm::min((float)WIN_HEIGHT / (float)SCR_HEIGHT, (float)WIN_WIDTH / (float)SCR_WIDTH);
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }
        };
        // Render glyph texture over quad
        glBindVertexArray(textVAO);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, vertices, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Activate corresponding render state	
        glm::mat4 projection = glm::ortho(0.0f, (float)WIN_WIDTH, 0.0f, (float)WIN_HEIGHT);
        shader.use();
        shader.setVec3("textColor", color);
        shader.setInt("text", 2);
        shader.setBool("type", type);
        shader.setMat4("projection", projection);
        shader.setInt("size", size);

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glActiveTexture(GL_TEXTURE2);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))        if (text[c] == L'\n')
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    return 0;
}
void text::setUsingSize(unsigned int width, unsigned int height)
{
    WIN_HEIGHT = height;
    WIN_WIDTH = width;
}
int text::deleteVaoAndVbo()
{
    if (!loadType)
    {
        errorList.push_back(2);
        return -1;
    }
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    loadType = false;
    return 0;
}
int text::getError()
{
    if (errorList.size() == 0)
    {
        return 0;
    }
    int error = (int)errorList[0];
    errorList.erase(0, 1);
    return error;
}
std::string text::errorCodeToName(int errorCode)
{
    if (errorCode == 0)
    {
        return "No error.";
    }
    return list[errorCode - 1];
}
#endif