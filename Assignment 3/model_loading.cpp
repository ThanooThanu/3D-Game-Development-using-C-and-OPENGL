#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera
Camera camera(glm::vec3(0.0f, 5.0f, 15.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ============== Player (Car) State ==============
glm::vec3 playerPosition = glm::vec3(0.0f, -2.0f, 5.0f); // Start on the new ground level
glm::vec3 playerModelOffset = glm::vec3(0.0f, 0.2f, 0.0f);
glm::vec3 playerFront = glm::vec3(0.0f, 0.0f, -1.0f);
float playerYaw = 180.0f; // Rotated the starting position
float playerSpeed = 0.0f;
const float PLAYER_ACCELERATION = 8.0f;
const float PLAYER_TURN_SPEED = 100.0f;
const float FRICTION = 3.0f;
const float MAX_SPEED = 15.0f;

// ============== Game Objects ==============
struct GameObject {
    glm::vec3 position;
    float scale;
    float collisionRadius;
    Model* model;
};

std::vector<GameObject> obstacles;

// ============== Collision Detection ==============
bool checkCollision(const GameObject& one, const GameObject& two)
{
    float distance = glm::length(one.position - two.position);
    return distance < (one.collisionRadius + two.collisionRadius);
}

int main()
{
    // GLFW and GLAD setup...
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "City Driver", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    Shader ourShader("1.model_loading.vs", "1.model_loading.fs");

    // Load Models
    std::cout << "Loading models..." << std::endl;
    Model cityModel(FileSystem::getPath("resources/objects/City/city.obj"));
    Model carModel(FileSystem::getPath("resources/objects/Cars/Car_OBJ.obj"));
    Model barrelModel(FileSystem::getPath("resources/objects/Barrel/Barrels_OBJ.obj"));
    std::cout << "Models loaded successfully!" << std::endl;

    // Setup Game Objects
    GameObject player;
    player.model = &carModel;
    player.position = playerPosition;
    player.scale = 0.1f;
    player.collisionRadius = 1.8f;

    // Lowered the Y-position for all barrels to the new ground level
    obstacles.push_back({ glm::vec3(10.0f, -2.0f, -10.0f), 0.01f, 1.0f, &barrelModel });
    obstacles.push_back({ glm::vec3(-5.0f, -2.0f, 20.0f), 0.01f, 1.0f, &barrelModel });
    obstacles.push_back({ glm::vec3(10.0f, -2.0f, 15.0f), 0.01f, 1.0f, &barrelModel });
    obstacles.push_back({ glm::vec3(-5.0f, -2.0f, -10.0f), 0.01f, 1.0f, &barrelModel });
    obstacles.push_back({ glm::vec3(0.0f, -2.0f, 28.0f), 0.01f, 1.0f, &barrelModel });

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Update Player State
        if (playerSpeed > 0) playerSpeed -= FRICTION * deltaTime;
        else if (playerSpeed < 0) playerSpeed += FRICTION * deltaTime;
        if (playerSpeed > MAX_SPEED) playerSpeed = MAX_SPEED;
        if (playerSpeed < -MAX_SPEED / 2.0f) playerSpeed = -MAX_SPEED / 2.0f;
        glm::vec3 front;
        front.x = cos(glm::radians(playerYaw));
        front.y = 0.0f;
        front.z = sin(glm::radians(playerYaw));
        playerFront = glm::normalize(front);
        playerPosition += playerFront * playerSpeed * deltaTime;
        player.position = playerPosition;

        // Simple Floor Collision
        float groundLevel = -1.0f; // Matched to the new ground level
        if (playerPosition.y < groundLevel)
        {
            playerPosition.y = groundLevel;
            player.position.y = groundLevel;
        }

        // ====================== Update Camera (CLOSER) ======================
        glm::vec3 cameraTarget = playerPosition + playerModelOffset;
        glm::vec3 cameraPos = cameraTarget - playerFront * 8.0f + glm::vec3(0.0, 4.0, 0.0);
        camera.Position = cameraPos;
        camera.Front = glm::normalize(cameraTarget - cameraPos);

        // Collision Detection & Response
        for (auto& obstacle : obstacles)
        {
            if (checkCollision(player, obstacle))
            {
                std::cout << "CRASH! You hit a barrel." << std::endl;
                playerPosition -= playerFront * 0.2f;
                player.position = playerPosition;
                playerSpeed = 0.0f;
            }
        }

        // Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ourShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // Render the city
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
        ourShader.setMat4("model", model);
        cityModel.Draw(ourShader);

        // Render the player (car)
        model = glm::mat4(1.0f);
        model = glm::translate(model, playerPosition + playerModelOffset);
        model = glm::rotate(model, glm::radians(-playerYaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(player.scale));
        ourShader.setMat4("model", model);
        carModel.Draw(ourShader);

        // Render the obstacles (barrels)
        for (const auto& obstacle : obstacles)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, obstacle.position);
            model = glm::scale(model, glm::vec3(obstacle.scale));
            ourShader.setMat4("model", model);
            obstacle.model->Draw(ourShader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// Input and callback functions remain the same
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        playerSpeed += PLAYER_ACCELERATION * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        playerSpeed -= PLAYER_ACCELERATION * deltaTime;
    if (abs(playerSpeed) > 0.1f)
    {
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            playerYaw += PLAYER_TURN_SPEED * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            playerYaw -= PLAYER_TURN_SPEED * deltaTime;
    }
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { camera.ProcessMouseScroll(static_cast<float>(yoffset)); }