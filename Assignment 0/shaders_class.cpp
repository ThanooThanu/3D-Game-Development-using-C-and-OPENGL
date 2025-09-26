#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <learnopengl/shader_s.h>
#include <iostream>
#include <vector>
#include <cstdlib>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

struct Vertex {
    float x, y, z;
    float r, g, b;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

unsigned int VBO, VAO;
std::vector<Vertex> vertices;

int main()
{
    // GLFW init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Triangle Growth", NULL, NULL);
    if (!window) { std::cout << "Failed to create GLFW window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n"; return -1;
    }

    Shader shader("3.3.shader.vs", "3.3.shader.fs");

    // VAO / VBO setup
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        glBindVertexArray(VAO);
        if (!vertices.empty())
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

bool rPressedLastFrame = false;

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        vertices.clear();
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    }
    bool rPressed = glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS;
    if (rPressed && !rPressedLastFrame) {
        if (vertices.size() >= 3) {
            vertices.pop_back();
            vertices.pop_back();
            vertices.pop_back();

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
        }
    }
    rPressedLastFrame = rPressed;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Convert screen coords to NDC
        float ndcX = 2.0f * xpos / SCR_WIDTH - 1.0f;
        float ndcY = 1.0f - 2.0f * ypos / SCR_HEIGHT;

        if (vertices.empty())
        {
            // First triangle
            vertices.push_back({ ndcX, ndcY, 0.0f, 1.0f, 0.0f, 0.0f });        // red
            vertices.push_back({ ndcX + 0.1f, ndcY, 0.0f, 0.0f, 1.0f, 0.0f });  // green
            vertices.push_back({ ndcX, ndcY + 0.1f, 0.0f, 0.0f, 0.0f, 1.0f });  // blue
        }
        else
        {
            // New triangle sharing the last edge
            Vertex v1 = vertices[vertices.size() - 2];  // previous vertex 1
            Vertex v2 = vertices[vertices.size() - 1];  // previous vertex 2
            Vertex vnew = { ndcX, ndcY, 0.0f,
                           static_cast<float>(rand() % 100) / 100.0f,
                           static_cast<float>(rand() % 100) / 100.0f,
                           static_cast<float>(rand() % 100) / 100.0f };

            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(vnew);
        }

        // Update GPU
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
    }
}