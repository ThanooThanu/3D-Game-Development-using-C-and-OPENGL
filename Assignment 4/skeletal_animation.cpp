#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

enum AnimState {
	IDLE = 1,
	IDLE_WALK,
	WALK_IDLE,
	WALK,
	IDLE_JUMP,
	JUMP_IDLE,
	IDLE_DANCE,
	DANCE_IDLE
};

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("anim_model.vs", "anim_model.fs");


	// load models
	// -----------
	Model ourModel(FileSystem::getPath("resources/objects/mouse/mouse.dae"));

	Animation idleAnimation(FileSystem::getPath("resources/objects/mouse/Idle.dae"), &ourModel);
	Animation walkAnimation(FileSystem::getPath("resources/objects/mouse/Walking.dae"), &ourModel);
	Animation jumpAnimation(FileSystem::getPath("resources/objects/mouse/Jump.dae"), &ourModel);
	Animation danceAnimation(FileSystem::getPath("resources/objects/mouse/Dancing.dae"), &ourModel);

	Animator animator(&idleAnimation);
	enum AnimState charState = IDLE;
	float blendAmount = 0.0f;
	float blendRate = 0.055f; // You can change this to blend faster/slower

	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// State machine logic
		switch (charState) {
		case IDLE:
			// Check for transitions out of IDLE
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
				blendAmount = 0.0f;
				animator.PlayAnimation(&idleAnimation, &walkAnimation, animator.m_CurrentTime, 0.0f, blendAmount);
				charState = IDLE_WALK;
			}
			// 'J' key for Jump
			else if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
				blendAmount = 0.0f;
				animator.PlayAnimation(&idleAnimation, &jumpAnimation, animator.m_CurrentTime, 0.0f, blendAmount);
				charState = IDLE_JUMP;
			}
			// 'K' key for Dance
			else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
				blendAmount = 0.0f;
				animator.PlayAnimation(&idleAnimation, &danceAnimation, animator.m_CurrentTime, 0.0f, blendAmount);
				charState = IDLE_DANCE;
			}
			// printf("idle \n"); // uncomment for debugging
			break;

			// --- WALK STATES ---
		case IDLE_WALK:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&idleAnimation, &walkAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&walkAnimation, NULL, startTime, 0.0f, blendAmount);
				charState = WALK;
			}
			// printf("idle_walk \n");
			break;
		case WALK:
			animator.PlayAnimation(&walkAnimation, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (glfwGetKey(window, GLFW_KEY_UP) != GLFW_PRESS) {
				charState = WALK_IDLE;
			}
			// printf("walking\n");
			break;
		case WALK_IDLE:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&walkAnimation, &idleAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&idleAnimation, NULL, startTime, 0.0f, blendAmount);
				charState = IDLE;
			}
			// printf("walk_idle \n");
			break;

			// --- JUMP STATES ---
		case IDLE_JUMP:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&idleAnimation, &jumpAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&jumpAnimation, NULL, startTime, 0.0f, blendAmount);
				charState = JUMP_IDLE;
			}
			// printf("idle_jump\n");
			break;
		case JUMP_IDLE:
			// !! IMPORTANT !!: Adjust this time (0.7f) to match your Jump animation length
			if (animator.m_CurrentTime > 0.7f) {
				blendAmount += blendRate;
				blendAmount = fmod(blendAmount, 1.0f);
				animator.PlayAnimation(&jumpAnimation, &idleAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
				if (blendAmount > 0.9f) {
					blendAmount = 0.0f;
					float startTime = animator.m_CurrentTime2;
					animator.PlayAnimation(&idleAnimation, NULL, startTime, 0.0f, blendAmount);
					charState = IDLE;
				}
				// printf("jump_idle \n");
			}
			else {
				// printf("jumping \n");
			}
			break;
			
		case IDLE_DANCE:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&idleAnimation, &danceAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&danceAnimation, NULL, startTime, 0.0f, blendAmount);
				charState = DANCE_IDLE;
			}
			// printf("idle_dance\n");
			break;
		case DANCE_IDLE:
			// !! IMPORTANT !!: Adjust this time (2.5f) to match your Dance animation length
			if (animator.m_CurrentTime > 2.5f) {
				blendAmount += blendRate;
				blendAmount = fmod(blendAmount, 1.0f);
				animator.PlayAnimation(&danceAnimation, &idleAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
				if (blendAmount > 0.9f) {
					blendAmount = 0.0f;
					float startTime = animator.m_CurrentTime2;
					animator.PlayAnimation(&idleAnimation, NULL, startTime, 0.0f, blendAmount);
					charState = IDLE;
				}
				// printf("dance_idle \n");
			}
			else {
				// printf("dancing \n");
			}
			break;
		} // END switch (charState)

		animator.UpdateAnimation(deltaTime);

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		ourShader.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		auto transforms = animator.GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i)
			ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);


		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);

		// <-- ADJUST THESE VALUES!
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

		ourShader.setMat4("model", model);
		ourModel.Draw(ourShader);


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
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

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; // reversed since y-coordinates go from bottom to top

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}