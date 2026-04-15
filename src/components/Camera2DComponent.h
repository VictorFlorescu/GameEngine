#pragma once
#include "raylib.h"

struct Camera2DComponent
{
	// center of a 1280x720 window
	// TODO: make it adapt to any resolution
	Vector2 offset = { 640.0f, 360.0f };
	float zoom = 1.0f;

	// In case of multiple cameras (e.g., split screen or minimap),
	// this helps the renderer know which one to draw the main world with.
	bool isMain = true;
};