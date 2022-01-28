#include "app.h"

App::App()
{
	//set member variables
	mWindowWidth = settings::TARGET_WIDTH * 4;
	mWindowHeight = settings::TARGET_HEIGHT * 4;
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

	cam3D = camera::freecam(glm::vec3(3.0f, 0.0f, 2.0f));

	//sound = Audio("audio/main.mp3");
	//sound.loop();

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
	testMap =  Map("maps/testMap.tmx", *mRender);
	messages = testMap.getMapMessages();
	cam2D.setCameraRects(testMap.getCameraRects());
	cam2D.setCameraMapRect(testMap.getMapRect());

	player = Player(
		{
		Animation(mRender->LoadTexture("textures/runUp.png"), 100, 16),
		Animation(mRender->LoadTexture("textures/runDown.png"), 100, 16),
		Animation(mRender->LoadTexture("textures/runRight.png"), 100, 17, true),
		Animation(mRender->LoadTexture("textures/runRight.png"), 100, 17)},
		testMap.getPlayerSpawn()
	);

	msgManager = MessageManager(*mRender);

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

		player.Update(timer, input);
		for(unsigned int i = 0; i < messages.size(); i++)
		{
			//std::cout << "x: " << messages[i].rect.x << std::endl;
			if(gh::colliding(player.rect(), messages[i].rect))
			{
				for(const auto &s: messages[i].messages)
					msgManager.AddMessage(*mRender, s);
				messages.erase(messages.begin() + i--);
			}
		}

	if(input.Keys[GLFW_KEY_T])
	{
		msgManager.AddMessage(*mRender, 
		"hello this is a test for the message box with automatic new lines from input string, I hope this works! Now I adjust my size based on the length of the input string.");
			msgManager.AddMessage(*mRender, 
		"I am a new message!");
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
	testMap.Update(cam2D.getCameraArea());
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

	mRender->setViewMatrixAndFov(cam3D.getViewMatrix(), cam3D.getZoom());
	mRender->set2DViewMatrix(cam2D.getViewMat());

	mRender->begin2DDraw();

	msgManager.Draw(*mRender, cam2D.getCameraOffset());

	player.Draw(*mRender, cam2D.getCameraArea());

	testMap.Draw(*mRender);
	
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
