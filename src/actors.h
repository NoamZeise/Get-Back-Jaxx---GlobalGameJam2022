#ifndef ACTORS_H
#define ACTORS_H

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>

#include "vulkan-render/render.h"
#include "vulkan-render/texture_loader.h"
#include "animation.h"
#include "timer.h"
#include "input.h"

#include <array>

enum Direction
{
	Up = 0,
	Down = 1,
	Left = 2,
	Right = 3,
};

class Actor
{
public:
	Actor(std::array<Animation, 4> directionTextures, glm::vec2 position)
	{
		this->animations = directionTextures;
		currentFrame = directionTextures[Down].getFrame(0);
		this->spriteRect = glm::vec4(position.x, position.y, currentFrame.tex.dim.x, currentFrame.tex.dim.y);
	}
	Actor() {}

	virtual void Update(Timer &timer, Input &input);

	void Draw(Render &render, glm::vec4 cameraRect)
	{
		render.DrawQuad(currentFrame.tex, spriteMat, glm::vec4(1), currentFrame.textureOffset);
	}

	glm::vec2 getMid()
	{
		return glm::vec2(spriteRect.x + spriteRect.z/2, spriteRect.y + spriteRect.w/2);
	}
	glm::vec4 rect()
	{
		return spriteRect;
	}
	glm::vec4 getHitBox()
	{
		return hitbox;
	}

protected:
	Frame currentFrame;
	glm::vec2 prevDir; 
	std::array<Animation, 4> animations;
	glm::vec4 spriteRect;
	glm::vec4 hitbox;
	glm::mat4 spriteMat;
	glm::vec2 velocity;
};


class Player : public Actor
{
public:
	Player(std::array<Animation, 4> animations, glm::vec2 position) :
				Actor(animations, position)
	{
		
	}
	Player() : Actor() {}

	void Update(Timer &timer, Input &input) override;

};


class Enemy : public Actor
{
	Enemy(std::array<Animation, 4> animations, glm::vec2 position) :
				Actor(animations, position)
	{
		
	}

	Enemy() : Actor() {}

	void Update(Timer &timer, Input &input) override;
};





#endif