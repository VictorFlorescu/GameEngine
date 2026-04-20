#pragma once

struct Animator2D
{
	int columns = 1;
	int rows = 1;

	int currentFrame = 0;
	int totalFrames = 1;

	float frameTime = 0.1f; // how long each frame stays on screen (0.1s = 10FPS)
	float timer = 0.0f;

	bool isPlaying = true;
	bool loop = true;
};