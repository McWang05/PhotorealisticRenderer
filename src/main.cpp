#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Callback to resize the rendering viewport if the window is resized
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

const char *vertexShaderSource = "#version 450 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "uniform mat4 model;\n"
                                 "uniform mat4 view;\n"
                                 "uniform mat4 projection;\n"
                                 "void main()\n"
                                 "{\n"
                                 // Note: Matrix multiplication in GLSL must happen in this exact reverse order
                                 "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
                                 "}\0";

const char *fragmentShaderSource = "#version 450 core\n"
                                   "out vec4 FragColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n" // Orange color
                                   "}\n\0";

bool loadModel(const std::string &path, unsigned int &VAO, unsigned int &VBO, int &vertexCount)
{
    Assimp::Importer importer;
    // Read file and optimize geometry
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "Assimp Error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    std::vector<float> vertices;

    // Loop through all meshes in the file and combine them into one buffer
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[i];
        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            // We divide by 100.0f to scale the car down temporarily.
            // Otherwise, real-world car coordinates will clip outside the screen.
            vertices.push_back(mesh->mVertices[j].x);
            vertices.push_back(mesh->mVertices[j].y);
            vertices.push_back(mesh->mVertices[j].z);
        }
    }

    vertexCount = vertices.size() / 3;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    return true;
}

int main()
{
    // 1. Initialize GLFW and request OpenGL 4.5 Core Profile
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. Create the window context
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Photorealistic Automotive Renderer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 3. Initialize GLAD to load OS-specific OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable Depth Testing so overlapping geometry renders correctly
    glEnable(GL_DEPTH_TEST);

    // Compile Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Compile Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Link Shaders into a Program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Clean up raw shaders as they are now linked
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    unsigned int VBO, VAO;
    int vertexCount = 0;

    // Use forward slashes for file paths in C++
    if (!loadModel("D:/PhotorealisticRenderer/models/Nissan S30.obj", VAO, VBO, vertexCount))
    {
        std::cout << "Failed to load 3D model!" << std::endl;
        return -1;
    }

    // Turn on wireframe mode so we can actually see the model's geometry
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // 4. The Render Loop
    while (!glfwWindowShouldClose(window))
    {
        // Input processing
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Clear the screen with a dark background color
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. Projection Matrix (Field of View, Aspect Ratio, Near Clip, Far Clip)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

        // 2. View Matrix (Camera Position, Target Position, Up Direction)
        // Position the camera 5 units back on the Z axis and 2 units up on the Y axis
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 2.0f, 5.0f), // Where the camera is
            glm::vec3(0.0f, 0.0f, 0.0f), // What the camera is looking at (center of the car)
            glm::vec3(0.0f, 1.0f, 0.0f)  // Which way is "Up"
        );

        // 3. Model Matrix (Position, Rotation, Scale of the car itself)
        glm::mat4 model = glm::mat4(1.0f); // Initialize to identity matrix
        // Slowly rotate the car around the Y-axis using the current time
        model = glm::rotate(model, (float)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));

        glUseProgram(shaderProgram);

        // Find the memory locations of the uniforms in the shader
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

        // Pass the GLM matrices to the shader
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);

        // ----- YOUR RENDERING CODE WILL GO HERE -----

        // Swap the front and back buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up memory on exit
    glfwTerminate();
    return 0;
}