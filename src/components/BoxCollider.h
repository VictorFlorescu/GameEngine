#pragma once
#include "raylib.h"

struct BoxCollider
{
	float width = 64.0f;
	float height = 64.0f;
	Vector2 offset = { 0.0f, 0.0f };// Shift the collider relative to the entity's centet
	bool isTrigger = false; // If true, it detects overlap but doesn't cause a physical bounce

};