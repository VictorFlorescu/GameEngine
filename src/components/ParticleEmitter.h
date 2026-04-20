#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>

struct Particle
{
	Vector3 position;
	Vector3 velocity;
	float life = 0.0f;
	float maxLife = 1.0f;
	Color color;
};

struct ParticleEmitter
{
	std::vector<Particle> particles;
	bool isEmitting = true;
	float emitRate = 0.1f;
	float emitTimer = 0.0f;

	// config
	Color startColor = YELLOW;
	Color endColor = RED;
	float particleLifetime = 1.5f;
	float speed = 50.0f;
};