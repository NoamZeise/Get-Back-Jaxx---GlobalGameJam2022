#ifndef BULLET_H 
#define BULLET_H

#include <glm/glm.hpp>

#include "vulkan-render/render.h"
#include "vulkan-render/texture_loader.h"
#include "vulkan-render/vkhelper.h"
#include "timer.h"
#include "gamehelper.h"

class Bullet
{
public:
	Bullet(Resource::Texture texture, glm::vec2 position, glm::vec2 velocity)
	{
		this->texture = texture;
		this->rect = glm::vec4(position.x, position.y, texture.dim.x, texture.dim.y);
		this->velocity = velocity;
	}

	void Update(Timer &timer, std::vector<glm::vec4> &colliders)
	{
		this->timer += timer.FrameElapsed(); 
		hitTimer += timer.FrameElapsed();
		float prevX = rect.x;
		rect.x += velocity.x * timer.FrameElapsed();
		for(const auto &mRect: colliders)
		{
			if(gh::colliding(mRect, rect))
			{
				hit = true;
				rect.x = prevX;
				velocity.x *= -1;
				break;
			}
		}
		float prevY = rect.y;
		rect.y += velocity.y * timer.FrameElapsed();
		for(const auto &mRect: colliders)
		{
			if(gh::colliding(mRect, rect))
			{
				hit = true;
				rect.y = prevY;
				velocity.y *= -1;
				break;
			}
		}

		mat = vkhelper::calcMatFromRect(rect, 0);
	}

	void Draw(Render &render)
	{
		render.DrawQuad(texture, mat, colour, glm::vec4(0, 0, 1, 1));
	}

	void Reverse(glm::vec2 pos, glm::vec4 colour)
	{
		hit = true;
		if(hitTimer > hitDelay)
		{
			hitTimer = 0;
			velocity = glm::normalize(this->getMid() - pos) * (glm::length(velocity) * 1.2f);
			this->colour = colour;
			timer = 0;
		}
	}

	bool Dead()
	{
		return lifespan < timer;
	}

	bool Active()
	{
		return hit;
	}

	glm::vec4 getRect()
	{
		return rect;
	}

	glm::vec2 getMid()
	{
		return glm::vec2(rect.x + rect.z/2, rect.y + rect.w/2);
	}

protected:
	Resource::Texture texture;
	glm::vec4 rect;
	glm::mat4 mat;
	glm::vec2 velocity;
	float speed;
	glm::vec4 colour = glm::vec4(1.0f);

	float lifespan = 10000;
	float timer = 0;
	float hitDelay = 1000;
	float hitTimer = 1000;
	bool hit = false;
};

class Particle : public Bullet
{
	Particle(Resource::Texture texture, glm::vec2 position, glm::vec2 velocity)
		: Bullet(texture, position, velocity)
		{
			
		}
};



#endif