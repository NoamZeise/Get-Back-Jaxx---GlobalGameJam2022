#include "actors.h"



void Actor::Update(Timer &timer, std::vector<glm::vec4> &colliders)
	{
		glm::vec2 ogHitboxPos = glm::vec2(hitbox.x, hitbox.y);
		if(abs(velocity.x) + abs(velocity.y) > max_speed)
		{
			velocity.x /=1.5;
			velocity.y /=1.5;
		}
		pushTimer += timer.FrameElapsed();
		if(pushTimer < pushDelay)
		{
			velocity = pushDir;
		}
		collided = false;

		float prevX = hitbox.x;
		hitbox.x += velocity.x * timer.FrameElapsed();
		for(const auto &rect: colliders)
		{
			if(gh::colliding(rect, hitbox))
			{
				collided = true;
				hitbox.x = prevX;
				break;
			}
		}
		float prevY = hitbox.y;
		hitbox.y += velocity.y * timer.FrameElapsed();

		prevAnim = direction;
		for(const auto &rect: colliders)
		{
			if(gh::colliding(rect, hitbox))
			{
				collided = true;
				hitbox.y = prevY;
				break;
			}
		}
		if(pushTimer > pushDelay)
		{
		if(velocity == glm::vec2(0))
		{
			if(prevDir.x < 0)
				direction = AnimationType::Left;
			else if(prevDir.x > 0)
				direction = AnimationType::Right;
			else if(prevDir.y < 0)
				direction = AnimationType::Up;
			else if(prevDir.y > 0)
				direction = AnimationType::Down;
			currentFrame = animations[direction].getFrame(0);
		}
		else
		{
			if(velocity.x < 0)
				direction = AnimationType::Left;
			else if(velocity.x > 0)
				direction = AnimationType::Right;
			else if(velocity.y < 0)
				direction = AnimationType::Up;
			else if(velocity.y > 0)
				direction = AnimationType::Down;
			currentFrame = animations[direction].Play(timer);
			prevDir = velocity;
		}
		}

		spriteRect = hitbox;
		spriteRect -= hitboxOffset;

		spriteRect.z = currentFrame.size.x;
		spriteRect.w = currentFrame.size.y;

		colour = glm::vec4(1.0f);
		spriteMat = vkhelper::calcMatFromRect(spriteRect, 0);
	}

void Actor::Hurt(glm::vec2 hurtLoc)
	{
		if(pushTimer > invDelay)
		{
			pushTimer = 0;
			health -= 1;
			pushDir = glm::normalize(this->getMid() - hurtLoc) * (max_speed * pushFactor);
		}
	}

Player::Player(std::vector<Animation> animations, glm::vec2 position, Audio *audio) :
				Actor(animations, position, audio)
{
	health = 5;
	dirtyFootsteps = SoundEffectBank("audio/sfx/dirty-footsteps/", 400, 50, 0.7, audio);
	cleanFootsteps = SoundEffectBank("audio/sfx/clean-footsteps/", 400, 50, 0.7, audio);
	weaponSound = SoundEffectBank("audio/sfx/machette-swing/", 500, 50, 0.8, audio);
	prevAnim = AnimationType::Down;
	pushFactor = 2;

	hitboxOffset = glm::vec4(0, 32, -29, -32);
	hitbox += hitboxOffset;
}

void Player::Update(Timer &timer, Input &input, std::vector<glm::vec4> &colliders)
	{
		attackingTimer += timer.FrameElapsed();
		velocity = glm::vec2(0);
		if(input.Keys[GLFW_KEY_W])
		{
			velocity.y -= max_speed; 
		}
		if(input.Keys[GLFW_KEY_S])
		{
			velocity.y += max_speed; 
		}
		if(input.Keys[GLFW_KEY_A])
		{
			velocity.x -= max_speed; 
		}
		if(input.Keys[GLFW_KEY_D])
		{
			velocity.x += max_speed; 
		}
		if(input.Keys[GLFW_KEY_COMMA] && attackingTimer > attackingDelay)
		{
			attackingTimer = 0;
			attackingDelay = animations[AnimationType::Attack].getTotalDuration();
			weaponSound.PlayOnce();
		}

		if((velocity.x != 0 || velocity.y != 0))
		{
			dirtyFootsteps.Play(timer);
		}
		hitboxOffset = glm::vec4(0, 32, -29, -32);
		if(direction == AnimationType::Left)
			hitboxOffset = glm::vec4(18, 32, -29, -32);
		if(prevAnim != AnimationType::Left && direction == AnimationType::Left)
		{
		//	hitbox.x += 18;
		}
		if(prevAnim == AnimationType::Left && direction != AnimationType::Left)
		{
		//	hitbox.x -= 18;
		}

		Actor::Update(timer, colliders);

		if(attackingTimer < attackingDelay)
		{
			attackFrame = animations[AnimationType::Attack].Play(timer);
			switch(direction)
			{
				case AnimationType::Up:
					weaponMat = vkhelper::calcMatFromRect(
								glm::vec4(spriteRect.x, spriteRect.y - spriteRect.w/2,
									attackFrame.size.x, attackFrame.size.y),
									0);
					damageZone = glm::vec4(spriteRect.x, spriteRect.y - spriteRect.w/2,
									attackFrame.size.x, attackFrame.size.y);
					break;
				case AnimationType::Down:
				weaponMat = vkhelper::calcMatFromRect(
						glm::vec4(spriteRect.x, spriteRect.y + spriteRect.w - 20,
									attackFrame.size.x, attackFrame.size.y),
									180);
					damageZone = glm::vec4(spriteRect.x, spriteRect.y + spriteRect.w,
									attackFrame.size.x, attackFrame.size.y);
					break;
				case AnimationType::Left:
				weaponMat = vkhelper::calcMatFromRect(
						glm::vec4(spriteRect.x - attackFrame.size.x + 10, spriteRect.y,
									attackFrame.size.x, attackFrame.size.y),
									-90);
						damageZone = glm::vec4(spriteRect.x - attackFrame.size.x, spriteRect.y,
									attackFrame.size.x, attackFrame.size.y);
					break;
				case AnimationType::Right:
				weaponMat = vkhelper::calcMatFromRect(
						glm::vec4(spriteRect.x + attackFrame.size.x - 10, spriteRect.y + spriteRect.w/2,
									attackFrame.size.x, attackFrame.size.y),
									90);
				damageZone = glm::vec4(spriteRect.x + attackFrame.size.x, spriteRect.y + spriteRect.w/2,
									attackFrame.size.x, attackFrame.size.y);
					break;
			}
		}
		else
		{
			animations[AnimationType::Attack].Reset();
		}
	}

	void Player::Draw(Render &render, glm::vec4 cameraRect)
	{
		if(direction != AnimationType::Up)
		{
			if(attackingTimer < attackingDelay)
			{
				render.DrawQuad(attackFrame.tex, weaponMat, glm::vec4(1), attackFrame.textureOffset);
			}
			Actor::Draw(render, cameraRect);
		}
		else
		{
			Actor::Draw(render, cameraRect);
			if(attackingTimer < attackingDelay)
			{
				render.DrawQuad(attackFrame.tex, weaponMat, glm::vec4(1), attackFrame.textureOffset);
			}
		}
		//	render.DrawQuad(Resource::Texture(), vkhelper::getModelMatrix(hitbox, 0), glm::vec4(1), glm::vec4(0, 0, 1, 1));
	}

void Enemy::Update(Timer &timer, std::vector<glm::vec4> &colliders, glm::vec2 player)
{
	if(active)
	{
		spriteRect = hitbox;
		shootTimer += timer.FrameElapsed();
		if(collided)
			velocity = -velocity;
		Actor::Update(timer, colliders);
	}
}
