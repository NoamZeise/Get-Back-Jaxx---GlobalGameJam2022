#include "message.h"



MessageManager::MessageManager(Render &render, Audio *audio)
{
	font = render.LoadFont("textures/dogicapixel.otf");
	messageBox = render.LoadTexture("textures/msgBox.png");
	msgBoxOffset = glm::vec4(100, 30, messageBox.dim.x, messageBox.dim.y);
	
	paperUp = SoundEffectBank("audio/sfx/paper/pickup/", 100.0f, 10.0f, 0.9f, audio);
	paperDown = SoundEffectBank("audio/sfx/paper/putdown/", 100.0f, 10.0f, 0.9f, audio);
}

void MessageManager::Update(Timer &timer, Input &input)
{
	if(messages.size() > 0)
	if(input.Keys[GLFW_KEY_SPACE] && !prevInput.Keys[GLFW_KEY_SPACE])
	{
		if(messages.size() > 0)
			paperUp.PlayOnce();
		else
			paperDown.PlayOnce();
		messages.erase(messages.begin());
	}

	prevInput = input;
}

void MessageManager::Draw(Render &render, glm::vec2 camOffset)
{
	if(messages.size() > 0)
	{
		msgBoxOffset.w = (fontYOff*1.5) + (lineSpacing * messages[0].lines.size());
		render.DrawQuad(messageBox, vkhelper::calcMatFromRect(glm::vec4(
			msgBoxOffset.x + camOffset.x, msgBoxOffset.y + camOffset.y, msgBoxOffset.z, msgBoxOffset.w), 0), glm::vec4(1),
			glm::vec4(0, 0, 1, 1), false);
		for(unsigned int i = 0; i < messages[0].lines.size(); i++)
		{
			render.DrawString(font, messages[0].lines[i], 
				glm::vec2(msgBoxOffset.x + fontXOff + camOffset.x, msgBoxOffset.y + fontYOff +
				 ( i * lineSpacing) + camOffset.y),
			 	textSize, 0, glm::vec4(0.3, 0.2, 0.1, 1));
		}
	}
}

void MessageManager::AddMessage(Render &render, std::string msg)
{
	std::string msgLine = "";
	std::string lastWord = "";
	messages.push_back(Message());
	for(int i = 0; i < msg.length(); i++ )
	{
		if(msg[i] == ' ')
		{
			if(render.MeasureString(font, msgLine + lastWord, textSize) > msgBoxOffset.z - fontXOff*2)
			{
				messages.back().lines.push_back(msgLine);
				msgLine = lastWord.append(" ");
			}
			else
			{
				msgLine = msgLine.append(lastWord.append(" "));
			}
			lastWord = "";
		}
		else
		{
			lastWord += msg[i];
		}
	}
	messages.back().lines.push_back(msgLine.append(lastWord));
	paperUp.PlayOnce();
}