#ifndef SOUND_BANK_H
#define SOUND_BANK_H

#include <vector>
#include <string>

#include "audio.h"
#include "gamehelper.h"
#include "timer.h"

class SoundEffectBank
{
public:
	SoundEffectBank(std::string path, float delay, float random, float volume, Audio *audio)
	{
		this->audio = audio;
		int i = 1;
		while(gh::exists(path + std::to_string(i) + ".mp3"))
			sounds.push_back(path + std::to_string(i++) + ".mp3");

		this->baseDelay = delay;
		this->sTimer = delay;
		this->volume = volume;
		this->random = random;
	}
	SoundEffectBank() {}

	void Play(Timer &timer)
	{
		sTimer += timer.FrameElapsed();

		if(sTimer > delay && sounds.size() > 0)
		{
			audio->oneTime(sounds[current], volume + rand.Real() * 0.2);
			sTimer = 0;
			current = (int)(rand.PositiveReal() * sounds.size());
			delay = baseDelay + (int)(rand.Real() * random);
		}
	}

	void PlayOnce()
	{
		audio->oneTime(sounds[current], volume + rand.Real() * 0.2);
		current = (int)(rand.PositiveReal() * sounds.size());
	}

private:
	Audio* audio;
	std::vector<std::string> sounds;
	gh::Random rand;

	int current = 0;
	double baseDelay = 400;
	double delay = 400;
	double sTimer = 400;
	double random = 10;
	float volume = 0.5;
};




#endif