#include "camera.h"

namespace camera
{
	freecam::freecam(glm::vec3 position)
	{
		_position = position;
		calculateVectors();
	}

	glm::mat4 freecam::getViewMatrix()
	{
		if(viewUpdated)
		{
			view = glm::lookAt(_position, _position + _front, _up);
			viewUpdated = false;
			return view;
		}
		return view;
	}

	float freecam::getZoom()
	{
		return _zoom;
	}


	void freecam::update(Input &input, Input &prevInput, Timer &timer)
	{
		viewUpdated = true;
		//keyboard
		float velocity = _speed * timer.FrameElapsed();
		if(input.Keys[GLFW_KEY_W])
			_position += _front * velocity;
		if(input.Keys[GLFW_KEY_A])
			_position -= _right * velocity;
		if(input.Keys[GLFW_KEY_S])
			_position -= _front * velocity;
		if(input.Keys[GLFW_KEY_D])
			_position += _right * velocity;
		if(input.Keys[GLFW_KEY_SPACE])
			_position += _worldUp * velocity;
		if(input.Keys[GLFW_KEY_LEFT_SHIFT])
			_position -= _worldUp * velocity;

		//mouse
		_pitch   += (prevInput.Y - input.Y) * _sensitivity;
		_yaw 	 += (prevInput.X - input.X) * _sensitivity;
	
		if(_pitch > 89.0f)
			_pitch = 89.0f;
		if(_pitch < -89.0f)
		 _pitch = -89.0f;
		
		//scroll
		_zoom -= input.offset * timer.FrameElapsed();
		if(_zoom < 1.0f)
			_zoom = 1.0f;
		if(_zoom > 100.0f)
			_zoom = 100.0f;

		calculateVectors();
		//std::cout << "X:" << _position.x << " Y:" << _position.y << " Z:" << _position.z << std::endl;
	}
	
	void freecam::calculateVectors()
	{
		_front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
		_front.y = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
		_front.z = sin(glm::radians(_pitch));
		_front = glm::normalize(_front);

		_right = glm::normalize(glm::cross(_front, _worldUp));
		_up = glm::normalize(glm::cross(_right, _front));
	}

	void camera2D::Target(glm::vec2 focus, Timer &timer)
		{
				
			glm::vec2 transform = glm::vec2(0);

			if(mapRect.z == 0 && cameraRects.size() == 0)
			{
				transform.x = -focus.x;
				transform.y = -focus.y;
			}
			else if(cameraRects.size() == 0)
			{
				transform.x = getOffset(settings::TARGET_WIDTH, focus.x, mapRect.x, mapRect.z);
				transform.y = getOffset(settings::TARGET_HEIGHT, focus.y, mapRect.y, mapRect.w);
			}
			else
			{
				glm::vec4 newRect = glm::vec4(0);
				for(const auto& rect: cameraRects)
				{
					if(gh::contains(focus, rect))
					{ 
						newRect = rect;
						break;
					}
				}
				if(newRect == glm::vec4(0))
				{
					transform.x = -focus.x;
					transform.y = -focus.y;
				}
				else
				{
					if (newRect != currentRect)
						currentRect = newRect;
					transform.x = getOffset(settings::TARGET_WIDTH, focus.x, currentRect.x, currentRect.z);
					transform.y = getOffset(settings::TARGET_HEIGHT, focus.y, currentRect.y, currentRect.w);
				}

				if(mapRect != glm::vec4(0))
				{
					transform.x = getOffset(settings::TARGET_WIDTH, -transform.x, mapRect.x, mapRect.z);
					transform.y = getOffset(settings::TARGET_HEIGHT, -transform.y, mapRect.y, mapRect.w);
				}
			}

			if(previousOff.x != 0 || previousOff.y != 0)
			{
			previousOff += glm::vec2(((transform.x - previousOff.x)/CAM2D_FLOAT) * timer.FrameElapsed(),
				((transform.y - previousOff.y)/CAM2D_FLOAT)* timer.FrameElapsed());
			transform = previousOff;
			}

			cameraArea = glm::vec4(-transform.x - settings::TARGET_WIDTH/2, -transform.y - settings::TARGET_HEIGHT/2,
									 settings::TARGET_WIDTH, settings::TARGET_HEIGHT);
			previousOff = transform;
			offset = glm::translate(glm::mat4(1.0f), glm::vec3(transform.x + settings::TARGET_WIDTH/2,
				 					transform.y + settings::TARGET_HEIGHT/2, 0));
		}

} //namespace end