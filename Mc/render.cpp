/*
* GTX1060 
* 
* 
* 
* 
*/

#include <glad/glad.h>

extern "C"
{
    _declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
};


#include <GLFW/glfw3.h>

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <ft2build.h>

#include <irrKlang/irrKlang.h>

#include <learnopengl/truck.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/text.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <queue>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void processInput(GLFWwindow* window);

unsigned int loadTexture(char const* path);



// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

unsigned int WIN_WIDTH = 800;
unsigned int WIN_HEIGHT = 600;

unsigned int BUFFER_WIDTH = 2160;
unsigned int BUFFER_HEIGHT = 1080;


// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool mouseFree = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// create a color attachment texture
unsigned int textureColorbuffer;

// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
unsigned int RBO;

// framebuffer configuration
unsigned int framebuffer;


// 顶点数组
// --------
glm::vec3 position[100000];
glm::mat4 models[100000];

int main()
{
    // 加入声音引擎
    irrklang::ISoundEngine* engine = irrklang::createIrrKlangDevice();

    if (!engine)
        return -1; // error starting up the engine

    //engine->play2D("WHITE_NIGHT.flac", true);

    // 初始化GLFW
    // ----------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


    // 创建一个窗口
    // ------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -2;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    // glad: 加载OpenGL函数指针
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -3;
    }

    // 设置后处理、文字着色器，开启深度测试
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    Shader textShader("text.vs", "text.fs");
    Shader screenShader("5.1.framebuffers_screen.vs", "5.1.framebuffers_screen.fs");
    
    text text("msyh.ttc", SCR_WIDTH, SCR_HEIGHT);
    text.loadText(window);

    // 告诉GLFW怎么显示鼠标指针
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 设置回调函数
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // 创建着色器
    // -------------------------
    Shader rightShader("9.1.geometry_shader.vs", "9.1.geometry_shader.fs", "9.1.right_shader.gs");
    Shader leftShader("9.1.geometry_shader.vs", "9.1.geometry_shader.fs", "9.1.left_shader.gs");
    Shader topShader("9.1.geometry_shader.vs", "9.1.geometry_shader.fs", "9.1.top_shader.gs");
    Shader bottomShader("9.1.geometry_shader.vs", "9.1.geometry_shader.fs", "9.1.bottom_shader.gs");
    Shader frontShader("9.1.geometry_shader.vs", "9.1.geometry_shader.fs", "9.1.front_shader.gs");
    Shader backShader("9.1.geometry_shader.vs", "9.1.geometry_shader.fs", "9.1.back_shader.gs");

    //计算向量和模型矩阵
    for (int i = 0; i < 100000; i++)
    {
        position[i] = glm::vec3(0.0f, i * 2, 0.0f);
        models[i] = glm::translate(glm::mat4(1.0f), position[i]);
    }

    float quadVertices[] = 
    { 
        // 位置       // 纹理坐标
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    //绑定
    unsigned int VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(models), &models, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(1 * sizeof(glm::vec4)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    // 屏幕VAO、VBO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


    
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, BUFFER_WIDTH, BUFFER_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO); // now actually attach it
    
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        return -4;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    

    unsigned int texture = loadTexture("container2.png");

    leftShader.setInt("textureA", 0);
    rightShader.setInt("textureA", 0);
    topShader.setInt("textureA", 0);
    bottomShader.setInt("textureA", 0);
    backShader.setInt("textureA", 0);
    frontShader.setInt("textureA", 0);

    int Fps = 100;
    bool FPSflag = false;
    int lowFps = -1;
    int highFps = 0;
    int lastSettlement = 1;

    std::wstring text_str = L"无数据";
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // 计算FPS
        // -------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // 输入
        // -----
        processInput(window);

        // 渲染
        // ------
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glViewport(0, 0, BUFFER_WIDTH, BUFFER_HEIGHT);

        // 绑定帧缓存
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 设置视图、投影矩阵
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // 画点
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        rightShader.use();
        rightShader.setMat4("projection", projection);
        rightShader.setMat4("view", view);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, 100000);
        
        leftShader.use();
        leftShader.setMat4("projection", projection);
        leftShader.setMat4("view", view);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, 100000);

        topShader.use();
        topShader.setMat4("projection", projection);
        topShader.setMat4("view", view);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, 100000);

        bottomShader.use();
        bottomShader.setMat4("projection", projection);
        bottomShader.setMat4("view", view);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, 100000);

        frontShader.use();
        frontShader.setMat4("projection", projection);
        frontShader.setMat4("view", view);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, 100000);

        backShader.use();
        backShader.setMat4("projection", projection);
        backShader.setMat4("view", view);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, 100000);

        
        // 更新显示的FPS
        if (lowFps <= 0)
        {
            lowFps = (int)(1.0f / deltaTime);
        }
        else if (lowFps > 1.0f / deltaTime) 
        {
            lowFps = (int)(1.0f / deltaTime);
        }

        if (highFps < 1.0f / deltaTime)
        {
            highFps = (int)(1.0f / deltaTime);
        }

        if (glfwGetTime() >= lastSettlement + 1 && !FPSflag)
        {
            FPSflag = true;
            Fps = (int)(1.0f / deltaTime);
            text_str = std::to_wstring(Fps) + L"  最低: " + std::to_wstring(lowFps) + L"  最高: " + std::to_wstring(highFps);
            lowFps = -1;
            highFps = 0;
            lastSettlement = (int)floor(glfwGetTime());
        }
        if (glfwGetTime() < lastSettlement + 1)
        {
            FPSflag = false;
        }
        // 渲染文字
        text.setUsingSize(WIN_WIDTH, WIN_HEIGHT);

        text.RenderText(textShader, text_str, 1.0f, 585.0f, 0.25f, glm::vec3(0.5, 0.8f, 0.2f));
        text.RenderText(textShader, L"你好，世界！", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.2f, 0.2f), true, 32);

        // 绑定回原本的帧缓存
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); // 禁用深度测试
        
        // 清理缓存
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.use();
        glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    
    
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteFramebuffers(1, &framebuffer);

    // delete engine
    engine->drop(); 

    text.deleteVaoAndVbo();

    glfwTerminate();

    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    //if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        //glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        mouseFree = true;
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        mouseFree = false;
    }
       
        

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    
    if (width != 0 && height != 0)
    {
        WIN_HEIGHT = height;
        WIN_WIDTH = width;

        //BUFFER_WIDTH = WIN_WIDTH / 3;
        //BUFFER_HEIGHT = WIN_HEIGHT / 3;

        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, BUFFER_WIDTH, BUFFER_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO); // now actually attach it

        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    }
    firstMouse = true;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;
    if (!mouseFree)
        camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format{};
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    //if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        
}