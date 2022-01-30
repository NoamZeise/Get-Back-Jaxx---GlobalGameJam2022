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
#include <string>
#include <atomic>
#include <array>

#include "vulkan-render/render.h"
#include "vulkan-render/config.h"
#include "input.h"
#include "audio.h"
#include "timer.h"
#include "camera.h"
#include "map.h"
#include "animation.h"
#include "message.h"
#include "actors.h"
#include "bullet.h"
#include "soundBank.h"
//#define TIME_APP_DRAW_UPDATE
//#define MULTI_UPDATE_ON_SLOW_DRAW

struct AssetBank
{
	std::vector<Animation> playerAnim;
	std::vector<Animation> enemy1Anim;
	std::vector<Animation> oldDoor;
	std::vector<Animation> newDoor;
	Resource::Texture scientist;
	Resource::Texture bullet;
	Resource::Texture items;
	Audio reactorHum;
	Audio backInTime;
	SoundEffectBank reactorHiss;
	SoundEffectBank waterDrops;
	Map forgottenMap;
	Map remeberedMap;
};

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

	void LoadMap(Map &map);

	glm::vec2 correctedPos(glm::vec2 pos);
	glm::vec2 appToScreen(glm::vec2 pos);

	glm::vec2 correctedMouse();
	
	GLFWwindow* mWindow;
	Render* mRender;
	int mWindowWidth, mWindowHeight;
	Input previousInput;
	Timer timer;
	camera::camera2D cam2D;

	std::thread submitDraw;
	std::atomic<bool> finishedDrawSubmit = true;

	float time = 0.0f;

	MessageManager msgManager;
	Audio music;
	Audio audio;
	Map currentMap;
	std::vector<MapMessage> messages;
	std::vector<Enemy> enemies;
	Player player;
	AssetBank assets;
	std::vector<glm::vec4> staticColliders;
	std::vector<glm::vec4> nonGapColliders;

	std::vector<Bullet> bullets;
	std::vector<Door> doors;
	bool inReactor = false;

	int itemCount = 0;
};

#endif
