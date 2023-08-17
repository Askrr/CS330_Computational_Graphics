#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    layout (location = 2) in vec3 aNormal;

    out vec2 TexCoord;
    out vec3 FragPos;
    out vec3 Normal;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        TexCoord = aTexCoord;
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
    }

)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    in vec3 FragPos;
    in vec3 Normal;
    out vec4 FragColor;

    uniform sampler2D ourTexture;
    uniform vec3 lightPos;
    uniform vec3 viewPos;

    void main()
    {
        vec3 objectColor = texture(ourTexture, TexCoord).rgb;
    
        vec3 lightDir = normalize(lightPos - FragPos);
        vec3 viewDir = normalize(viewPos - FragPos);

        float diff = max(dot(normalize(Normal), lightDir), 0.0);
        vec3 diffuse = diff * objectColor;

        FragColor = vec4(diffuse, 1.0);
}
)";

glm::vec3 cameraPosition = glm::vec3(0.0f, 0.5f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
bool firstMouse = true;
float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMouseCallback(GLFWwindow* window, double xpos, double ypos);

int main(int argc, char* argv[])
{
    GLFWwindow* window = nullptr;
    if (!UInitialize(argc, argv, &window))
        return EXIT_FAILURE;
    // Coffee cup vertex data
    float coffeeCupVertices[] = {
        // Positions           // Texture Coords
        0.5f, 0.0f, 0.5f,     0.0f, 0.0f, // Bottom left
        0.7f, 0.0f, 0.5f,     1.0f, 0.0f, // Bottom right
        0.7f, 0.0f, 0.7f,     1.0f, 1.0f, // Top right
        0.5f, 0.0f, 0.7f,     0.0f, 1.0f, // Top left

        0.6f, 0.1f, 0.6f,     0.5f, 1.0f, // Top
    };
    // Computer monitor vertex data (square screen)
    float monitorVertices[] = {
        // Positions           // Texture Coords
        -0.25f, 0.0f, 0.25f,  0.0f, 0.0f, // Bottom left
         0.25f, 0.0f, 0.25f,  1.0f, 0.0f, // Bottom right
         0.25f, 0.0f, -0.25f, 1.0f, 1.0f, // Top right
        -0.25f, 0.0f, -0.25f, 0.0f, 1.0f, // Top left
    };
    // Plane vertex data
    float planeVertices[] = {
        // Positions             // Colors
        -0.5f,  0.0f, -0.5f - 0.05f,   1.0f, 0.0f, 0.0f,  // Top left
         0.5f,  0.0f, -0.5f - 0.05f,   0.0f, 1.0f, 0.0f,  // Top right
         0.5f,  0.0f,  0.5f - 0.05f,   0.0f, 0.0f, 1.0f,  // Bottom right

        -0.5f,  0.0f, -0.5f - 0.05f,   1.0f, 0.0f, 0.0f,  // Top left
         0.5f,  0.0f,  0.5f - 0.05f,   0.0f, 0.0f, 1.0f,  // Bottom right
        -0.5f,  0.0f,  0.5f - 0.05f,   0.0f, 1.0f, 0.0f   // Bottom left
    };

    // Cube vertex data
    float cubeVertices[] = {
        // Positions           // Texture Coords
        -0.1f, -0.1f, -0.1f,   0.0f, 0.0f, // Bottom left
        0.1f, -0.1f, -0.1f,    1.0f, 0.0f, // Bottom right
        0.1f, 0.1f, -0.1f,     1.0f, 1.0f, // Top right
        -0.1f, 0.1f, -0.1f,    0.0f, 1.0f, // Top left

        -0.1f, -0.1f, 0.1f,    0.0f, 0.0f, // Bottom left
        0.1f, -0.1f, 0.1f,     1.0f, 0.0f, // Bottom right
        0.1f, 0.1f, 0.1f,      1.0f, 1.0f, // Top right
        -0.1f, 0.1f, 0.1f,     0.0f, 1.0f, // Top left

        -0.1f, -0.1f, -0.1f,   0.0f, 0.0f, // Bottom left
        0.1f, -0.1f, -0.1f,    1.0f, 0.0f, // Bottom right
        0.1f, -0.1f, 0.1f,     1.0f, 1.0f, // Top right
        -0.1f, -0.1f, 0.1f,    0.0f, 1.0f, // Top left

        -0.1f, 0.1f, -0.1f,    0.0f, 0.0f, // Bottom left
        0.1f, 0.1f, -0.1f,     1.0f, 0.0f, // Bottom right
        0.1f, 0.1f, 0.1f,      1.0f, 1.0f, // Top right
        -0.1f, 0.1f, 0.1f,     0.0f, 1.0f, // Top left

        -0.1f, -0.1f, -0.1f,   0.0f, 0.0f, // Bottom left
        0.1f, -0.1f, -0.1f,    1.0f, 0.0f, // Bottom right
        0.1f, 0.1f, -0.1f,     1.0f, 1.0f, // Top right
        -0.1f, 0.1f, -0.1f,    0.0f, 1.0f, // Top left

        -0.1f, -0.1f, 0.1f,    0.0f, 0.0f, // Bottom left
        0.1f, -0.1f, 0.1f,     1.0f, 0.0f, // Bottom right
        0.1f, 0.1f, 0.1f,      1.0f, 1.0f, // Top right
        -0.1f, 0.1f, 0.1f,     0.0f, 1.0f, // Top left


        // Translate the plane along the negative z-axis to separate it from the cubes
        -0.5f,  0.0f, -0.5f - 0.01f,   0.5f, 0.5f, 0.5f,  // Top left
        0.5f,  0.0f, -0.5f - 0.01f,   0.5f, 0.5f, 0.5f,  // Top right
        0.5f,  0.0f,  0.5f - 0.01f,   0.5f, 0.5f, 0.5f,  // Bottom right

        -0.5f,  0.0f, -0.5f - 0.01f,   0.5f, 0.5f, 0.5f,  // Top left
         0.5f,  0.0f,  0.5f - 0.01f,   0.5f, 0.5f, 0.5f,  // Bottom right
        -0.5f,  0.0f,  0.5f - 0.01f,   0.5f, 0.5f, 0.5f,  // Bottom left   
    };

    // Create VAO and VBO for plane
    GLuint planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    // Bind VAO and VBO for plane
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);

    // Upload vertex data to the VBO for plane
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    // Specify vertex attribute pointers for plane
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Create VAO and VBO for cubes
    GLuint cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    // Bind VAO and VBO for cubes
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

    // Upload vertex data to the VBO for cubes
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Specify vertex attribute pointers for cubes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Create and compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Create and compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Create shader program and attach shaders for plane
    GLuint planeShaderProgram = glCreateProgram();
    glAttachShader(planeShaderProgram, vertexShader);
    glAttachShader(planeShaderProgram, fragmentShader);
    glLinkProgram(planeShaderProgram);

    // Create shader program and attach shaders for cubes
    GLuint cubeShaderProgram = glCreateProgram();
    glAttachShader(cubeShaderProgram, vertexShader);
    glAttachShader(cubeShaderProgram, fragmentShader);
    glLinkProgram(cubeShaderProgram);

    // Set up projection matrix
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glm::mat4 projection = glm::perspective(glm::radians(fov), static_cast<float>(width) / height, 0.1f, 100.0f);

    // Load the texture
    unsigned int texture;
    int textureWidth, textureHeight, nrChannels;
    stbi_set_flip_vertically_on_load(true); // Flip the image vertically (optional, depending on how the image is stored)
    unsigned char* data = stbi_load("sugar.jpg", &textureWidth, &textureHeight, &nrChannels, 0);

    if (!data)
    {
        std::cout << "Failed to load texture" << std::endl;
        // Handle error if image loading fails
        return EXIT_FAILURE;
    }

    if (textureWidth == 0 || textureHeight == 0)
    {
        std::cout << "Failed to get texture width or height" << std::endl;
        stbi_image_free(data);
        return EXIT_FAILURE;
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    glfwSetCursorPosCallback(window, UMouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window))
    {
        UProcessInput(window);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Set the light position and view position
        GLint lightPosLoc = glGetUniformLocation(planeShaderProgram, "lightPos");
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(glm::vec3(1.0f, 2.0f, 2.0f)));

        GLint viewPosLoc = glGetUniformLocation(planeShaderProgram, "viewPos");
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPosition));

        // Bind VAO and VBO for cubes
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

        // Upload vertex data to the VBO for cubes
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

        // Specify vertex attribute pointers for cubes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);
        // Set the projection matrix for both shaders
        GLint projectionLoc = glGetUniformLocation(planeShaderProgram, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        GLint projectionLoc2 = glGetUniformLocation(cubeShaderProgram, "projection");
        glUniformMatrix4fv(projectionLoc2, 1, GL_FALSE, glm::value_ptr(projection));

        // Set the view matrix for both shaders
        glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
        GLint viewLoc = glGetUniformLocation(planeShaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        GLint viewLoc2 = glGetUniformLocation(cubeShaderProgram, "view");
        glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view));

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Set the texture uniform for the cube shader program
        glUseProgram(cubeShaderProgram);
        glUniform1i(glGetUniformLocation(cubeShaderProgram, "ourTexture"), 0);

        // Draw the plane with lighting
        glUseProgram(planeShaderProgram);
        glm::mat4 planeModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, 0.0f));
        GLint modelLoc = glGetUniformLocation(planeShaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(planeModel));
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw the 5 cubes with lighting
        glUseProgram(cubeShaderProgram);
        for (int i = 0; i < 5; ++i)
        {
            glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f + i * 0.5f, 0.1f, -0.5f));
            GLint modelLoc2 = glGetUniformLocation(cubeShaderProgram, "model");
            glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(cubeModel));
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // Draw the coffee cup
        glUseProgram(cubeShaderProgram);
        glm::mat4 coffeeCupModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, 1.0f)); // Adjust the position
        GLint coffeeCupModelLoc = glGetUniformLocation(cubeShaderProgram, "model");
        glUniformMatrix4fv(coffeeCupModelLoc, 1, GL_FALSE, glm::value_ptr(coffeeCupModel));
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 18); // 6 vertices * 3 triangles
        // Draw the computer monitor
        glUseProgram(cubeShaderProgram);
        glm::mat4 monitorModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f)); // Adjust the position
        GLint monitorModelLoc = glGetUniformLocation(cubeShaderProgram, "model");
        glUniformMatrix4fv(monitorModelLoc, 1, GL_FALSE, glm::value_ptr(monitorModel));
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 18); // 6 vertices * 3 triangles

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(planeShaderProgram);
    glDeleteProgram(cubeShaderProgram);

    glfwTerminate();

    return 0;
}

bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    *window = glfwCreateWindow(800, 600, "Plane with Cubes", nullptr, nullptr);

    if (*window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(*window);
    glewInit();
    glViewport(0, 0, 800, 600);

    // Add these lines to enable depth testing and set the clear color
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set clear color to black

    return true;
}

void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void UMouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void UProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPosition += glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraPosition -= glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed;
}