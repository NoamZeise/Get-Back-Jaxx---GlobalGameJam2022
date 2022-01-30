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
#include "gamehelper.h"
#include "soundBank.h"

#include <array>

enum AnimationType
{
	Up = 0,
	Down = 1,
	Left = 2,
	Right = 3,
	Attack = 4,
};

class Actor
{
public:
	Actor(std::vector<Animation> animations, glm::vec2 position, Audio *audio)
	{
		this->animations = animations;
		direction = AnimationType::Down;
		currentFrame = animations[Down].getFrame(0);
		this->spriteRect = glm::vec4(position.x, position.y, currentFrame.size.x, currentFrame.size.y);
		this->hitbox = spriteRect;
		this->audio = audio;
	}
	Actor() {}

	virtual void Update(Timer &timer, std::vector<glm::vec4> &colliders);

	virtual void Draw(Render &render, glm::vec4 cameraRect)
	{
		if(gh::colliding(spriteRect, cameraRect))
		{
		if(pushTimer < pushDelay)
			colour = glm::vec4(1, 0, 0, 1);
		render.DrawQuad(currentFrame.tex, spriteMat, colour, currentFrame.textureOffset);
		}
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
	AnimationType getDirection()
	{
		return direction;
	}

	virtual void Hurt(glm::vec2 hurtLoc);

	bool Alive()
	{
		return health > 0 || pushTimer < pushDelay;
	}

	void Reset(glm::vec2 position)
	{
		spriteRect.x = position.x;
		spriteRect.y = position.y;
		hitbox = spriteRect;
		velocity = glm::vec2(0);
		health = max_health;
		pushTimer = pushDelay +1;
		collided = false;
	}

protected:
	Frame currentFrame;
	glm::vec2 prevDir; 
	std::vector<Animation> animations;
	glm::vec4 spriteRect;
	glm::vec4 hitbox;
	glm::mat4 spriteMat;
	glm::vec2 velocity = glm::vec2(0);
	AnimationType direction;
	AnimationType prevAnim;

	int health = 3;
	int max_health = 3;
	float max_speed = 0.1f;
	bool collided = false;

	float pushFactor = 10;
	float pushTimer = 150;
	float pushDelay = 150;
	float invDelay = 300;
	glm::vec2 pushDir = glm::vec2(0);
	Audio* audio;
	glm::vec4 colour = glm::vec4(1.0f);
	glm::vec4 hitboxOffset = glm::vec4(0, 0, 0, 0);

};


class Player : public Actor
{
public:
	Player(std::vector<Animation> animations, glm::vec2 position, Audio* audio);
	Player( ): Actor() {}

	void Update(Timer &timer, Input &input, std::vector<glm::vec4> &colliders);

	void Draw(Render &render, glm::vec4 cameraRect) override;

	glm::vec4 getDamageRect()
	{
		if(attackingDelay < attackingTimer)
			return glm::vec4(0);
		else
			return damageZone;
	}

private:
	float attackingDelay = 100;
	float attackingTimer = 0;
	Frame attackFrame;
	glm::vec4 damageZone = glm::vec4(0);
	glm::mat4 weaponMat;

	SoundEffectBank dirtyFootsteps;
	SoundEffectBank cleanFootsteps;
	SoundEffectBank weaponSound;
};


class Enemy : public Actor
{
public:
	Enemy(std::vector<Animation> animations, glm::vec2 position, Audio *audio):
				Actor(animations, position, audio)
	{
		max_speed = 0.03f;
		velocity.x = max_speed;
		stabbed = SoundEffectBank("audio/sfx/stab/wet/", 100, 10, 0.4, audio);
		shootDelay = baseShootDelay + (rand.Real() * 1000);
	}

	Enemy() : Actor() {}

	void Hurt(glm::vec2 hurtLoc) override
	{
		if(pushTimer > invDelay)
		{
			stabbed.PlayOnce();
		}
		Actor::Hurt(hurtLoc);
	}

	virtual void Update(Timer &timer, std::vector<glm::vec4> &colliders, glm::vec2 player);

	virtual bool Shoot()
	{
		if(shootTimer > shootDelay)
			{
			shootTimer = 0;
			shootDelay = baseShootDelay + (rand.Real() * 1000);
			return true;
		}
		float left = shootDelay - shootTimer;
		if(left < 500 && left > 1)
		{
			float close = (left)/500;
			colour = glm::vec4(close,close, close, 1);
		}
		return false;
	}


	bool active = true;
protected:
	SoundEffectBank stabbed; 
	float shootDelay = 3000;
	float baseShootDelay = 3000;
	gh::Random rand;
	float shootTimer = 0;
};


class Scientist : public Enemy
{
public:
	Scientist(Resource::Texture texture, glm::vec2 position, Audio *audio):
				Enemy({Animation(texture, 200, 22), Animation(texture, 200, 22), Animation(texture, 200, 22), Animation(texture, 200, 22)}, position, audio)
	{
		stabbed = SoundEffectBank("audio/sfx/stab/dry/", 100, 10, 0.4, audio);
		shootDelay = 0;
		direction = AnimationType::Up;
	}

	bool Shoot() override
	{
		return false;
	}

	void Update(Timer &timer, std::vector<glm::vec4> &colliders, glm::vec2 player) override
	{
		shootDelay = 0;
		direction = AnimationType::Up;
		spriteRect = hitbox;
		velocity = glm::vec2(0, 0);
		Actor::Update(timer, colliders);
		currentFrame = animations[AnimationType::Up].getFrame(0);
	}
};

class Door: public Enemy
{
	public:
	Door(std::vector<Animation> animations, glm::vec2 position, Audio *audio):
				Enemy(animations, position, audio)
	{
		shootDelay = 0;
		open = SoundEffectBank("audio/sfx/door/open/", 100, 10, 0.4, audio);
		close = SoundEffectBank("audio/sfx/door/close/", 100, 10, 0.4, audio);
	}

	bool Shoot() override
	{
		return false;
	}

	void Update(Timer &timer, std::vector<glm::vec4> &colliders, glm::vec2 player) override
	{
		
		spriteRect = hitbox;
		velocity = glm::vec2(0, 0);
		float dist = abs(hitbox.x - player.x) + abs(hitbox.y - player.y);
		bool inRange = (dist < range);
		if(inRange && !lastInRange)
		{
			open.PlayOnce();
		}
		if(!inRange && lastInRange)
		{
			close.PlayOnce();
		}

		if(inRange)
		{
			currentFrame = animations[AnimationType::Up].PlayOnce(timer);
			animations[AnimationType::Down].Reset();
		}
		else
		{
			currentFrame = animations[AnimationType::Down].PlayOnce(timer);
			animations[AnimationType::Up].Reset();
		}
		lastInRange = inRange;

				colour = glm::vec4(1.0f);
		spriteMat = vkhelper::calcMatFromRect(spriteRect, 0);
	}

	void Hurt(glm::vec2 hurtLoc) override
	{
		
	}
private:
	glm::vec2 lastPlayerPos = glm::vec2(0);
	float range = 150;
	bool lastInRange = false;
	SoundEffectBank open;
	SoundEffectBank close;
};




#endif