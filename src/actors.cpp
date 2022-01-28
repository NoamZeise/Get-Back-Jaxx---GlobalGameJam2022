#include "actors.h"



void Actor::Update(Timer &timer, Input &input)
	{
		spriteRect.x += velocity.x * timer.FrameElapsed();
		spriteRect.y += velocity.y * timer.FrameElapsed();

		if(velocity == glm::vec2(0))
		{
			if(prevDir.x < 0)
				currentFrame = animations[Left].getFrame(0);
			else if(prevDir.x > 0)
				currentFrame = animations[Right].getFrame(0);
			else if(prevDir.y < 0)
				currentFrame = animations[Up].getFrame(0);
			else if(prevDir.y > 0)
				currentFrame = animations[Down].getFrame(0);
		}
		else
		{
			if(velocity.x < 0)
				currentFrame = animations[Left].Play(timer);
			if(velocity.x > 0)
				currentFrame = animations[Right].Play(timer);
			if(velocity.y < 0)
				currentFrame = animations[Up].Play(timer);
			if(velocity.y > 0)
				currentFrame = animations[Down].Play(timer);
			prevDir = velocity;
		}

		spriteRect.z = currentFrame.size.x;
		spriteRect.w = currentFrame.size.y;

		hitbox = spriteRect;

		spriteMat = vkhelper::calcMatFromRect(spriteRect, 0);
	}

void Player::Update(Timer &timer, Input &input)
	{
		velocity = glm::vec2(0);
		if(input.Keys[GLFW_KEY_W])
		{
			velocity.y -= 0.1f; 
		}
		if(input.Keys[GLFW_KEY_S])
		{
			velocity.y += 0.1f; 
		}
		if(input.Keys[GLFW_KEY_A])
		{
			velocity.x -= 0.1f; 
		}
		if(input.Keys[GLFW_KEY_D])
		{
			velocity.x += 0.1f; 
		}


		Actor::Update(timer, input);
		hitbox.w /=2;
		hitbox.y += hitbox.w;
	}

void Enemy::Update(Timer &timer, Input &input)
{
	
}
