#pragma once
#include <string>

struct AudioSource
{
	std::string soundPath; // key into AssetManager
	float volume = 1.0f;
	float pitch = 1.0f;
	bool loop = false;
	bool playOnStart = false;

	// internal playback state - managed by AudioSystem

	bool m_started = false;
	bool m_playing = false;
};