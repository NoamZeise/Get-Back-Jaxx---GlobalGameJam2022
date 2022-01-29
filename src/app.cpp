#include "app.h"

App::App()
{
	//set member variables
	mWindowWidth = settings::TARGET_WIDTH * 2;
	mWindowHeight = settings::TARGET_HEIGHT * 2;
	//init glfw window
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
			throw std::runtime_error("failed to initialise glfw!");
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //using vulkan not openGL
	mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, "GGJ22", nullptr, nullptr);
	if(!mWindow)
	{
		glfwTerminate();
		throw std::runtime_error("failed to create glfw window!");
	}
	
	GLFWimage winIcon[1];
	winIcon[0].pixels = stbi_load("textures/icon.png", &winIcon[0].width, &winIcon[0].height, 0, 4); //rgba channels 
	glfwSetWindowIcon(mWindow, 1, winIcon);
	
	glfwSetWindowUserPointer(mWindow, this);
	glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);
	glfwSetCursorPosCallback(mWindow, mouse_callback);
	glfwSetScrollCallback(mWindow, scroll_callback);
	glfwSetKeyCallback(mWindow, key_callback);
	glfwSetMouseButtonCallback(mWindow, mouse_button_callback);
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glfwRawMouseMotionSupported())
    	glfwSetInputMode(mWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	
	if(settings::FIXED_RATIO)
		glfwSetWindowAspectRatio(mWindow, settings::TARGET_WIDTH, settings::TARGET_HEIGHT);

	mRender = new Render(mWindow, glm::vec2(settings::TARGET_WIDTH, settings::TARGET_HEIGHT));
	loadAssets();

}

App::~App()
{
	if(submitDraw.joinable())
		submitDraw.join();
	delete mRender;
	mRender = nullptr;
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void App::loadAssets()
{
	currentMap =  Map("maps/forgottenLab.tmx", *mRender);

	assets.playerAnim = {
		Animation(mRender->LoadTexture("textures/sprites/player/runUp.png"), 100, 16),
		Animation(mRender->LoadTexture("textures/sprites/player/runDown.png"), 100, 16),
		Animation(mRender->LoadTexture("textures/sprites/player/runRight.png"), 100, 17, true),
		Animation(mRender->LoadTexture("textures/sprites/player/runRight.png"), 100, 17),
		Animation(mRender->LoadTexture("textures/sprites/player/weapon.png"), 20, 15)
		};
	assets.enemy1Anim =  {
		Animation(mRender->LoadTexture("textures/sprites/enemy1/up-22.png"), 200, 22),
		Animation(mRender->LoadTexture("textures/sprites/enemy1/down-22.png"), 200, 22),
		Animation(mRender->LoadTexture("textures/sprites/enemy1/right-18.png"), 200, 18, true),
		Animation(mRender->LoadTexture("textures/sprites/enemy1/right-18.png"), 200, 18),
		Animation(mRender->LoadTexture("textures/sprites/enemy1/right-18.png"), 200, 18)
	};

	assets.bullet = mRender->LoadTexture("textures/sprites/bullet.png");
	msgManager = MessageManager(*mRender);

	LoadMap(currentMap);

	mRender->endResourceLoad();
}

void App::run()
{
	while (!glfwWindowShouldClose(mWindow))
	{
		update();
		if(mWindowWidth != 0 && mWindowHeight != 0)
			draw();
	}
}

void App::resize(int windowWidth, int windowHeight)
{
	if(submitDraw.joinable())
		submitDraw.join();
	mWindowWidth = windowWidth;
	mWindowHeight = windowHeight;
	if(mRender != nullptr && mWindowWidth != 0 && mWindowHeight != 0)
		mRender->framebufferResized = true;
}

void App::LoadMap(Map &map)
{
	bullets.clear();
	messages.clear();
	messages = currentMap.getMapMessages();
	music.stop();
	music = Audio(currentMap.getMusic());
	music.loop();
	music.setVolume(0.3);
	staticColliders.clear();
	staticColliders = currentMap.getGapColliders();
	nonGapColliders.clear();
	nonGapColliders = currentMap.getMapColliders();
	staticColliders.insert(staticColliders.end(), nonGapColliders.begin(), nonGapColliders.end());
	cam2D.SetCameraOffset(map.getPlayerSpawn());
	cam2D.setCameraRects(currentMap.getCameraRects());
	cam2D.setCameraMapRect(currentMap.getMapRect());
	player = Player(assets.playerAnim, currentMap.getPlayerSpawn(), &audio);
	enemies.clear();
	auto eSpawns = currentMap.getEnemySpawns();
	for(const auto &e: eSpawns)
	{
		if(e.type == EnemyTypes::Basic)
			enemies.push_back(Enemy(assets.enemy1Anim, e.spawn, &audio));
	}
}

void App::update()
{
#ifdef TIME_APP_DRAW_UPDATE
	auto start = std::chrono::high_resolution_clock::now();
#endif
	glfwPollEvents();

	if(msgManager.isActive())
	{
		msgManager.Update(timer, input);
	}
	else
	{

		player.Update(timer, input, staticColliders);

		auto playerMid =  player.getMid();
		for(unsigned int i = 0; i < enemies.size(); i++)
		{
			enemies[i].Update(timer, staticColliders, playerMid);
			if(gh::colliding(enemies[i].getHitBox(), cam2D.currentRoom))
			{
				enemies[i].active = true;
			}
			else
				enemies[i].active = false;
			if(enemies[i].active)
			{
				if(gh::colliding(enemies[i].getHitBox(), player.getDamageRect()))
					enemies[i].Hurt(player.getMid());
				if(gh::colliding(enemies[i].getHitBox(), player.getHitBox()))
					player.Hurt(enemies[i].getMid());

				for(unsigned int j = 0; j < bullets.size(); j++)
				{
					if(bullets[j].Active())
					{
						if(gh::colliding(enemies[i].getHitBox(), bullets[j].getRect()))
						{
							enemies[i].Hurt(bullets[j].getMid());
							bullets.erase(bullets.begin() + j--);
						}
					}
				}

				if(enemies[i].Shoot())
				{
					bullets.push_back(Bullet(
						assets.bullet, 
						enemies[i].getMid(),
						glm::normalize(player.getMid() - enemies[i].getMid()) * 0.1f));
				}
			}
			if(!enemies[i].Alive())
				enemies.erase(enemies.begin() + i--);
		}
		for(unsigned int i = 0; i < bullets.size(); i++)
		{
			bullets[i].Update(timer, nonGapColliders);
			if(gh::colliding(bullets[i].getRect(), player.getDamageRect()))
			{
				bullets[i].Reverse(player.getMid(), glm::vec4(1));
				bullets.push_back(bullets[i]);
				bullets.erase(bullets.begin() + i--);
				continue;
			}
			if(gh::colliding(bullets[i].getRect(), player.getHitBox()))
			{
				player.Hurt(bullets[i].getMid());
				bullets.erase(bullets.begin() + i--);
				continue;
			}
			for(unsigned int j = 0; j < bullets.size(); j++)
			{
				if(i == j)
					continue;
				if(gh::colliding(bullets[i].getRect(), bullets[j].getRect()))
				{
					bullets[i].Reverse(bullets[j].getMid(), glm::vec4(1));
					bullets[j].Reverse(bullets[i].getMid(), glm::vec4(1));
				}
			}
			if(bullets[i].Dead())
				bullets.erase(bullets.begin() + i--);
		}

		if(!player.Alive())
			LoadMap(currentMap);

		for(unsigned int i = 0; i < messages.size(); i++)
			if(gh::colliding(player.rect(), messages[i].rect))
			{
				for(const auto &s: messages[i].messages)
					msgManager.AddMessage(*mRender, s);
				messages.erase(messages.begin() + i--);
			}

	}

	postUpdate();
#ifdef TIME_APP_DRAW_UPDATE
	auto stop = std::chrono::high_resolution_clock::now();
	std::cout 
		 << "update: "
         << std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count() 
		 << " microseconds" << std::endl;
#endif
}

void App::postUpdate()
{
	time += timer.FrameElapsed();
	cam2D.Target(player.getMid(), timer);
	currentMap.Update(cam2D.getCameraArea());
	timer.Update();
	previousInput = input;
	input.offset = 0;
}


void App::draw()
{
#ifdef TIME_APP_DRAW_UPDATE
	auto start = std::chrono::high_resolution_clock::now();
#endif

#ifdef MULTI_UPDATE_ON_SLOW_DRAW
	if(!finishedDrawSubmit)
		return;
	finishedDrawSubmit = false;
#endif
	if(submitDraw.joinable())
		submitDraw.join();

	mRender->set2DViewMatrix(cam2D.getViewMat());

	mRender->begin2DDraw();

	//mRender->DrawQuad(Resource::Texture(), vkhelper::calcMatFromRect(cam2D.currentRoom, 0), glm::vec4(1));

	msgManager.Draw(*mRender, cam2D.getCameraOffset());

	player.Draw(*mRender, cam2D.getCameraArea());

	for(auto &e: enemies)
		e.Draw(*mRender, cam2D.getCameraArea());
	for(auto &b: bullets)
		b.Draw(*mRender);

	currentMap.Draw(*mRender);
	
	submitDraw = std::thread(&Render::endDraw, mRender, std::ref(finishedDrawSubmit));

#ifdef TIME_APP_DRAW_UPDATE
	auto stop = std::chrono::high_resolution_clock::now();
	std::cout 
	<< "draw: " 
	<< std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count() 
	<< " microseconds" << std::endl;
#endif
}

glm::vec2 App::correctedPos(glm::vec2 pos)
{
	return glm::vec2(pos.x * ((float)settings::TARGET_WIDTH / (float)mWindowWidth), pos.y * ((float)settings::TARGET_HEIGHT / (float)mWindowHeight));
}

glm::vec2 App::correctedMouse()
{
	return correctedPos(glm::vec2(input.X, input.Y)); 
}

#pragma region GLFW_CALLBACKS


void App::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	app->resize(width, height);
}

void App::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	app->input.X = xpos;
	app->input.Y = ypos;
}
void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	app->input.offset = yoffset;
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	if (key == GLFW_KEY_F && action == GLFW_RELEASE)
	{
		if (glfwGetWindowMonitor(window) == nullptr)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(window, NULL, 100, 100, app->mWindowWidth, app->mWindowHeight, mode->refreshRate);
		}
	}
	if(key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			app->input.Keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			app->input.Keys[key] = false;
		}
	}
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));

	if (button >= 0 && button < 8)
	{
		if (action == GLFW_PRESS)
		{
			app->input.Buttons[button] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			app->input.Buttons[button] = true;
		}
	}
}

void App::error_callback(int error, const char* description)
{
    throw std::runtime_error(description);
}

#pragma endregion
