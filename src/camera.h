#ifndef FREE_CAMERA_H
#define FREE_CAMERA_H

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
#include <vector>

#include "input.h"
#include "timer.h"
#include "gamehelper.h"
#include "vulkan-render/config.h"

namespace camera
{

	static float getOffset(float target, float focus, float roomPos, float roomSize)
	{
		if(target > roomSize)
			return -roomSize/2 -roomPos;
		float min = roomPos + target/2;
		float max = roomPos + roomSize - target/2;
		if(focus > min && focus < max)
			return -focus;
		if(focus < min)
			return -min;
		if(focus > max)
			return -max;
		return -focus;
	}

	class freecam
	{
	public:
		freecam() { _position = glm::vec3(0.0f, 0.0f, 0.0f); };
		freecam(glm::vec3 position);
		glm::mat4 getViewMatrix();
		float getZoom();
		void update(Input &input, Input &prevInput, Timer &timer);
		

	private:
		glm::vec3 _position;
		glm::vec3 _front;
		glm::vec3 _up;
		glm::vec3 _right;
		glm::vec3 _worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		bool viewUpdated = true;

		float _yaw = 200.0f;
		float _pitch = -20.0f;

		float _speed = 0.01f;
		float _sensitivity = 0.05f;
		float _zoom = 45.0f;


		void calculateVectors();
	};

	const unsigned int CAM2D_FLOAT = 250;
	class camera2D
	{
	public:
		void SetCameraOffset(glm::vec2 pos)
		{
			cameraArea = glm::vec4(pos.x - settings::TARGET_WIDTH/2, pos.y - settings::TARGET_HEIGHT/2,
			 settings::TARGET_WIDTH, settings::TARGET_HEIGHT);
			offset = glm::translate(glm::mat4(1.0f),
				 glm::vec3(-pos.x + settings::TARGET_WIDTH/2, -pos.y + settings::TARGET_HEIGHT/2, 0));
		}

		void Target(glm::vec2 focus, Timer &timer);


		glm::mat4 getViewMat()
		{
			return offset;
		}
		void setCameraRects(std::vector<glm::vec4> cameraRects)
		{
			this->cameraRects = cameraRects;
		}
		void setCameraMapRect(glm::vec4 mapRect)
		{
			this->mapRect = mapRect;
		}
		void clearCameraRects()
		{
			cameraRects.clear();
			mapRect = glm::vec4(0);
		}

		glm::vec4 getCameraArea()
		{
			return cameraArea;
		}

		glm::vec2 getCameraOffset()
		{
			return glm::vec2(cameraArea.x, cameraArea.y);
		}
	private:
		glm::mat4 offset = glm::mat4(1.0f);
		std::vector<glm::vec4> cameraRects;
		glm::vec4 mapRect =  glm::vec4(0);

		glm::vec4 currentRect = glm::vec4(0, 0, 0, 0);
		glm::vec2 previousOff = glm::vec2(0);

		glm::vec4 cameraArea = glm::vec4(0);
	};



}	//namesapce end



#endif