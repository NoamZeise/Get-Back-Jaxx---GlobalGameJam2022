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
	mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, "Get Back Jaxx", nullptr, nullptr);
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
	assets.forgottenMap =  Map("maps/forgottenLab.tmx", *mRender);
	assets.forgottenMap.name = "forgotten";
	assets.remeberedMap =  Map("maps/rememberedLab.tmx", *mRender);
	assets.remeberedMap.name = "remebered";

	assets.playerAnim = {
		Animation(mRender->LoadTexture("textures/sprites/player/character-right-51-walk.png"), 200, 51),
		Animation(mRender->LoadTexture("textures/sprites/player/character-right-51-walk.png"), 200, 51),
		Animation(mRender->LoadTexture("textures/sprites/player/character-right-51-walk.png"), 200, 51, true),
		Animation(mRender->LoadTexture("textures/sprites/player/character-right-51-walk.png"), 200, 51),
		Animation(mRender->LoadTexture("textures/sprites/player/weapon.png"), 20, 45)
		};
	assets.enemy1Anim =  {
		Animation(mRender->LoadTexture("textures/sprites/enemy1/walk.png"), 200, 71),
		Animation(mRender->LoadTexture("textures/sprites/enemy1/walk.png"), 200, 71),
		Animation(mRender->LoadTexture("textures/sprites/enemy1/walk.png"), 200, 71),
		Animation(mRender->LoadTexture("textures/sprites/enemy1/walk.png"), 200, 71, true),
		Animation(mRender->LoadTexture("textures/sprites/enemy1/walk.png"), 200, 71)
	};

	assets.scientist = mRender->LoadTexture("textures/sprites/enemy1/down-22.png");
	assets.newDoor = {
		Animation(mRender->LoadTexture("textures/new_door_anim.png"), 300, 32),
		Animation(mRender->LoadTexture("textures/new_door_anim.png"), 300, 32, false, true) };
	assets.oldDoor = {
		Animation(mRender->LoadTexture("textures/old_door_anim.png"), 300, 32),
		Animation(mRender->LoadTexture("textures/old_door_anim.png"), 300, 32, false, true) };
	assets.bullet = mRender->LoadTexture("textures/sprites/bullet.png");
	msgManager = MessageManager(*mRender, &audio);

	currentMap = assets.forgottenMap;
	LoadMap(currentMap);
	messages.clear();
	messages = currentMap.getMapMessages();

	assets.reactorHiss = SoundEffectBank("audio/sfx/reactorA/pressure/", 10000.0f, 5000.0f, 0.9f, &audio);
	assets.reactorHum = Audio("audio/sfx/reactorA/hum/1.mp3");
	assets.waterDrops = SoundEffectBank("audio/sfx/water-droplet/", 8000.0f, 3000.0f, 0.1f, &audio);

	assets.items = mRender->LoadTexture("textures/collect_items.png");

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
	music.stop();
	music = Audio(currentMap.getMusic());
	music.loop();
	music.setVolume(0.3);
	if(map.name == "forgotten")
	{
		mRender->setLightingProps(0.001f, 0.0006f);
	}
	else
	{
		mRender->setLightingProps(0.005f, 0.0001f);
	}
	staticColliders.clear();
	staticColliders = currentMap.getGapColliders();
	nonGapColliders.clear();
	nonGapColliders = currentMap.getMapColliders();
	staticColliders.insert(staticColliders.end(), nonGapColliders.begin(), nonGapColliders.end());
	cam2D.SetCameraOffset(map.getPlayerSpawn());
	cam2D.setCameraRects(currentMap.getCameraRects());
	cam2D.setCameraMapRect(currentMap.getMapRect());
	if(currentMap.lastCheckpoint != glm::vec4(0))
		player = Player(assets.playerAnim, currentMap.lastCheckpoint, &audio);
	else
		player = Player(assets.playerAnim, currentMap.getPlayerSpawn(), &audio);
	player.Update(timer, input, staticColliders);
	enemies.clear();
	auto eSpawns = currentMap.getEnemySpawns();
	for(const auto &e: eSpawns)
	{
		if(e.type == EnemyTypes::Basic)
			enemies.push_back(Enemy(assets.enemy1Anim, e.spawn, &audio));
		enemies.back().Update(timer, staticColliders, player.getMid());
		enemies.back().active = false;
	}

	for(const auto &d: map.doors)
	{
		if(map.name == "forgotten")
			doors.push_back(Door(assets.oldDoor, d, &audio));
		else
			doors.push_back(Door(assets.newDoor, d, &audio));
		//enemies.back().Update(timer, staticColliders, player.getMid());
		//enemies.back().active = false;
	}
	for(const auto &s: map.scientist)
	{
		enemies.push_back(Scientist(assets.scientist, s, &audio));
		enemies.back().Update(timer, staticColliders, player.getMid());
		enemies.back().active = false;
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
		
		assets.waterDrops.Play(timer);
		player.Update(timer, input, staticColliders);
		if(gh::colliding(currentMap.getReactorRoom(), player.rect()))
		{
			assets.reactorHiss.Play(timer);
			if(!inReactor)
			{
				assets.reactorHum.loop();
				assets.reactorHum.setVolume(1.0f);
				music.pause();
			}
			inReactor = true;
		}
		else
		{
			if(inReactor)
			{
				music.play();
							assets.reactorHum.stop();
			}
			inReactor = false;
		}
		
		
		for(unsigned int i = 0; i < currentMap.checkpoints.size(); i++)
		{
			if(gh::colliding(currentMap.checkpoints[i], player.getHitBox()))
			{
				currentMap.lastCheckpoint = currentMap.checkpoints[i];
			}
		}
		
		auto playerMid =  player.getMid();
		for(unsigned int i = 0; i < enemies.size(); i++)
		{
			enemies[i].Update(timer, staticColliders, playerMid);
			if(!enemies[i].active)
			{
				if(gh::colliding(enemies[i].getHitBox(), cam2D.currentRoom))
				{
					enemies[i].active = true;
				}
			}
			else
			{
				if(gh::colliding(enemies[i].getHitBox(), player.getDamageRect()))
					enemies[i].Hurt(playerMid);
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

				if(currentMap.name == "forgotten" && enemies[i].Shoot())
				{
					bullets.push_back(Bullet(
						assets.bullet, 
						enemies[i].getMid(),
						glm::normalize(playerMid - enemies[i].getMid()) * 0.1f));
				}
			}
			if(!enemies[i].Alive())
				enemies.erase(enemies.begin() + i--);
		}

		for(auto &d: doors)
		{
			d.Update(timer, staticColliders, playerMid);
		}
		
		for(unsigned int i = 0; i < bullets.size(); i++)
		{
			bullets[i].Update(timer, nonGapColliders);
			if(gh::colliding(bullets[i].getRect(), player.getDamageRect()))
			{
				bullets[i].Reverse(playerMid, glm::vec4(1));
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

		bool messageAdded = false;
		for(unsigned int i = 0; i < messages.size(); i++)
			if(gh::colliding(player.getHitBox(), messages[i].rect))
			{
				for(const auto &s: messages[i].messages)
				{
					msgManager.AddMessage(*mRender, s);
					messageAdded = true;
				}
				messages.erase(messages.begin() + i--);
			}
			if(!messageAdded)
			{
				for(unsigned int i = 0; i < currentMap.items.size(); i++)
				{
					if(gh::colliding(currentMap.items[i], player.getHitBox()))
					{
						itemCount++;
						if(currentMap.lastCheckpoint == glm::vec4(0))
							player.Reset(currentMap.getPlayerSpawn());
						else
						{
							player.Reset(
								glm::vec2(currentMap.lastCheckpoint.x + currentMap.lastCheckpoint.z/2,
											currentMap.lastCheckpoint.y + currentMap.lastCheckpoint.w/2));
						}

						for(unsigned int e = 0; e < enemies.size(); e++)
						{
							if(enemies[e].active)
								enemies.erase(enemies.begin() + e--);
						}
					}
				}
			if(itemCount >= 4 || (currentMap.name != "forgotten"))
			{
			if(gh::colliding(player.getHitBox(), currentMap.getReactorTP()))
			{
				if(currentMap.name == assets.forgottenMap.name)
				{
					assets.backInTime = Audio("audio/music/BackInTime.mp3");
					assets.backInTime.setVolume(0.7f);
					assets.backInTime.play();
					currentMap = assets.remeberedMap;
					LoadMap(currentMap);
					messages.clear();
					messages = currentMap.getMapMessages();
				}
				else
				{
					glfwSetWindowShouldClose(mWindow, true);
				}
			}
		}
		}
		
		

	}

	std::vector<glm::vec2> lights;
	glm::vec4 camExpanded = cam2D.getCameraArea();
	camExpanded.x -= 300;
	camExpanded.y -= 300;
	camExpanded.z += 600;
	camExpanded.w += 600;
	glm::vec2 playerSc = player.getMid();
	playerSc.x -= cam2D.getCameraOffset().x;
	playerSc.y -= cam2D.getCameraOffset().y;
	lights.push_back(appToScreen(playerSc));
	for(auto &b: bullets)
	{
		glm::vec2 screenCoords = b.getMid();
		screenCoords.x -= cam2D.getCameraOffset().x;
		screenCoords.y -= cam2D.getCameraOffset().y;
		lights.push_back(appToScreen(screenCoords));
	}
	for(const auto &l: currentMap.lights)
	{
		if(gh::contains(l, camExpanded))
		{
			glm::vec2 screenCoords = l;
			screenCoords.x -= cam2D.getCameraOffset().x;
			screenCoords.y -= cam2D.getCameraOffset().y;
					
			lights.push_back(appToScreen(screenCoords));
		}
	}
	mRender->setLights(lights);

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

	if(itemCount > 0)
		mRender->DrawQuad(assets.items, vkhelper::calcMatFromRect(
		glm::vec4(cam2D.getCameraOffset().x, cam2D.getCameraOffset().y,
		 (assets.items.dim.x/4) * itemCount, assets.items.dim.y), 0),
		glm::vec4(1.0f),
		vkhelper::calcTexOffset(
			assets.items.dim,
			glm::vec4(-5, 0, (assets.items.dim.x/4) * itemCount, assets.items.dim.y)),false);


	msgManager.Draw(*mRender, cam2D.getCameraOffset());

	player.Draw(*mRender, cam2D.getCameraArea());

	for(auto &e: enemies)
		e.Draw(*mRender, cam2D.getCameraArea());
	for(auto &b: bullets)
		b.Draw(*mRender);

	for(auto &d: doors)
		{
			d.Draw(*mRender, cam2D.getCameraArea());
		}
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

glm::vec2 App::appToScreen(glm::vec2 pos)
{
	return 
	glm::vec2(pos.x * ((float)mWindowWidth / (float)settings::TARGET_WIDTH),
	          pos.y * ((float)mWindowHeight / (float)settings::TARGET_HEIGHT));
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
