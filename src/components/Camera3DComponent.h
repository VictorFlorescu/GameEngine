#pragma once
#include "raylib.h"

struct Camera3DComponent
{
	Vector3 target = { 0.0f, 0.0f, 0.0f };
	Vector3 up = { 0.0f, 1.0f, 0.0f };
	float fovy = 45.0f;
	int projection = CAMERA_PERSPECTIVE;
	bool isMain = true;

	float sensitivity = 0.1f;
	bool invertX = false;
	bool invertY = false;
};