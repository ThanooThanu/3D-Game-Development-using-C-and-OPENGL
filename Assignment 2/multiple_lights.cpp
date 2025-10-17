#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <iostream>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 15.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// A structure to hold unique animation properties for each cube
struct CubeKineticProps {
    glm::vec3 basePosition;
    glm::vec3 motionAmplitude; // How far it moves on each axis
    glm::vec3 motionFrequency; // How fast it moves on each axis
    float motionOffset;      // Ensures cubes don't all move in sync
};

int main()
{
    // === Standard OpenGL/GLFW Initialization (similar to original) ===
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Kinetic Sculpture", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // === Shader Program Compilation ===
    Shader lightingShader("6.multiple_lights.vs", "6.multiple_lights.fs");
    Shader lightCubeShader("6.light_cube.vs", "6.light_cube.fs");

    // === Vertex Data (Unchanged) ===
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    // === NEW: Procedurally generate the kinetic sculpture cubes ===
    std::vector<CubeKineticProps> kineticCubes;
    int gridSize = 5; // Create a 5x5x5 grid of cubes
    float spacing = 2.0f;
    for (int x = -gridSize; x <= gridSize; ++x) {
        for (int y = -gridSize; y <= gridSize; ++y) {
            for (int z = -gridSize; z <= gridSize; ++z) {
                CubeKineticProps props;
                // Calculate base position in the grid
                props.basePosition = glm::vec3(x * spacing, y * spacing, z * spacing);

                // The further from the center, the more it moves
                float distanceFromCenter = glm::length(props.basePosition);
                props.motionAmplitude = glm::vec3(distanceFromCenter * 0.1f, distanceFromCenter * 0.15f, distanceFromCenter * 0.1f);

                // Varied frequencies for more chaotic, interesting movement
                props.motionFrequency = glm::vec3(0.5f + x * 0.05f, 0.5f + y * 0.05f, 0.5f + z * 0.05f);

                // Offset ensures they don't all move together
                props.motionOffset = (x + y + z) * 0.2f;

                kineticCubes.push_back(props);
            }
        }
    }

    // Positions of the point lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.0f,  5.0f,  0.0f),
        glm::vec3(0.0f, -5.0f,  0.0f),
    };

    // Colors for the point lights
    glm::vec3 pointLightColors[] = {
        glm::vec3(1.0f, 0.2f, 0.2f), // Red
        glm::vec3(0.2f, 0.2f, 1.0f), // Blue
    };

    // Vertex Buffer and Array Object setup
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // TexCoords are still in the VBO but we won't use them for the metallic look
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    // glEnableVertexAttribArray(2);

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // === RENDER COMMANDS ===
        // Black background for drama
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);

        // NEW: Material properties for a metallic look
        lightingShader.setVec3("material.ambient", 0.1f, 0.1f, 0.1f);
        lightingShader.setVec3("material.diffuse", 0.8f, 0.8f, 0.8f); // Will be colored by lights
        lightingShader.setVec3("material.specular", 1.0f, 1.0f, 1.0f); // Strong highlight
        lightingShader.setFloat("material.shininess", 64.0f);

        // === LIGHTING SETUP (Revised for a more dramatic effect) ===
        // Directional light (a dim, cool "moonlight")
        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        lightingShader.setVec3("dirLight.ambient", 0.02f, 0.02f, 0.05f); // Very dim blue ambient
        lightingShader.setVec3("dirLight.diffuse", 0.1f, 0.1f, 0.15f);
        lightingShader.setVec3("dirLight.specular", 0.2f, 0.2f, 0.2f);

        // Animate the point lights to orbit the sculpture
        float lightOrbitRadius = 8.0f;
        pointLightPositions[0].x = sin(currentFrame * 0.5f) * lightOrbitRadius;
        pointLightPositions[0].z = cos(currentFrame * 0.5f) * lightOrbitRadius;
        pointLightPositions[1].x = sin(-currentFrame * 0.3f) * lightOrbitRadius;
        pointLightPositions[1].z = cos(-currentFrame * 0.3f) * lightOrbitRadius;

        // Point lights (dynamic, colored lights)
        for (int i = 0; i < 2; i++) {
            std::string number = std::to_string(i);
            lightingShader.setVec3("pointLights[" + number + "].position", pointLightPositions[i]);
            lightingShader.setVec3("pointLights[" + number + "].ambient", pointLightColors[i] * 0.05f);
            lightingShader.setVec3("pointLights[" + number + "].diffuse", pointLightColors[i] * 0.8f);
            lightingShader.setVec3("pointLights[" + number + "].specular", pointLightColors[i]);
            lightingShader.setFloat("pointLights[" + number + "].constant", 1.0f);
            lightingShader.setFloat("pointLights[" + number + "].linear", 0.09f);
            lightingShader.setFloat("pointLights[" + number + "].quadratic", 0.032f);
        }

        // SpotLight (the user's "flashlight")
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09f);
        lightingShader.setFloat("spotLight.quadratic", 0.032f);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        // View/Projection matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // === RENDER THE KINETIC SCULPTURE ===
        glBindVertexArray(cubeVAO);
        for (const auto& props : kineticCubes)
        {
            glm::mat4 model = glm::mat4(1.0f);

            // NEW: Apply the kinetic motion using sine waves
            float time = currentFrame + props.motionOffset;
            glm::vec3 motion;
            motion.x = sin(time * props.motionFrequency.x) * props.motionAmplitude.x;
            motion.y = cos(time * props.motionFrequency.y) * props.motionAmplitude.y;
            motion.z = sin(time * props.motionFrequency.z) * props.motionAmplitude.z;

            // Start with the base grid position and add the fluid motion
            glm::vec3 finalPosition = props.basePosition + motion;
            model = glm::translate(model, finalPosition);

            // Add a slow, continuous self-rotation
            float angle = currentFrame * 25.0f;
            model = glm::rotate(model, glm::radians(angle), glm::normalize(glm::vec3(0.5f, 1.0f, 0.7f)));

            // Make the cubes smaller to fit more in the view
            model = glm::scale(model, glm::vec3(0.5f));

            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Draw the light source cubes
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        glBindVertexArray(lightCubeVAO);
        for (unsigned int i = 0; i < 2; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.4f)); // Make lights bigger to see them
            lightCubeShader.setMat4("model", model);
            // Set light cube color to match the light it emits
            lightCubeShader.setVec3("lightColor", pointLightColors[i]);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

// === Callback and Input Functions (mostly unchanged) ===
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}