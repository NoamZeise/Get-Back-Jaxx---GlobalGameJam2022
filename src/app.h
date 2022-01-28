#ifndef APP_H
#define APP_H

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <thread>
#include <stdexcept>
#include <atomic>

#include "vulkan-render/render.h"
#include "vulkan-render/config.h"
#include "input.h"
#include "audio.h"
#include "timer.h"
#include "camera.h"
#include "map.h"
#include "animation.h"

//#define TIME_APP_DRAW_UPDATE
//#define MULTI_UPDATE_ON_SLOW_DRAW

class App
{
public:
	App();
	~App();
	void run();
	void resize(int windowWidth, int windowHeight);

#pragma region GLFW_CALLBACKS
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void error_callback(int error, const char* description);
#pragma endregion

	Input input;
private:
	void loadAssets();
	void update();
	void postUpdate();
	void draw();

	glm::vec2 correctedPos(glm::vec2 pos);
	glm::vec2 correctedMouse();
	
	GLFWwindow* mWindow;
	Render* mRender;
	int mWindowWidth, mWindowHeight;
	Input previousInput;
	Timer timer;
	camera::freecam cam3D;
	camera::camera2D cam2D;

	std::thread submitDraw;
	std::atomic<bool> finishedDrawSubmit = true;

	float time = 0.0f;

	Resource::Texture testTex;
	Resource::Font* testFont;
	glm::vec4 player = glm::vec4(10, 10, 16, 23);
	Animation walkDown;
	Animation walkRight;
	Animation walkUp;
	Animation walkLeft;
	glm::vec2 prevDir = glm::vec2(0);
	Frame currentFrame;
	
	Audio sound;
	Map testMap;
};

#endif
