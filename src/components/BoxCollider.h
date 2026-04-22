#pragma once
#include "raylib.h"
#include <functional>
#include "../ecs/Entity.h"

struct BoxCollider
{
	float width = 1.0f;
	float height = 1.0f;
	float depth = 1.0f;
	Vector3 offset = { 0.0f, 0.0f };// Shift the collider relative to the entity's centet
	bool isTrigger = false; // If true, it detects overlap but doesn't cause a physical bounce

	std::function<void(Entity me, Entity other)> onCollision = nullptr;

};