#ifndef MESSAGE_H
#define MESSAGE_H

#include "glm/glm.hpp"
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

#include "vulkan-render/texture_loader.h"
#include "vulkan-render/texfont.h"
#include "vulkan-render/render.h"
#include "vulkan-render/vkhelper.h"
#include "vulkan-render/config.h"

#include "timer.h"
#include "input.h"

#include <string>
#include <iostream>

struct Message
{
	std::vector<std::string> lines;
};

class MessageManager
{
public:
	MessageManager(Render &render);
	MessageManager() {}
	void Update(Timer &timer, Input &input);
	void Draw(Render &render, glm::vec2 camOffset);
	void AddMessage(Render &render, std::string message);

	bool isActive() {return messages.size() > 0; }

private:
	Resource::Font* font;
	Resource::Texture messageBox;

	bool done = true;
	std::vector<Message> messages;

	glm::vec4 msgBoxOffset = glm::vec4(0);
	float msgBoxWidth = 190;
	int textSize = 7;
	int lineSpacing = 10;

	int fontXOff = 10;
	int fontYOff = 20;

	Input prevInput;
};


#endif