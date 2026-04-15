#pragma once
#include "raylib.h"

enum class BodyType { Dynamic, Kinematic, Static};

struct Rigidbody
{
	Vector3 velocity = { 0,0,0 };
	Vector3 acceleration = { 0,0,0 };
	Vector3 angularVelocity = { 0,0,0 };

	float mass = 1.0f;
	float drag = 0.01f; // linear damping per second
	float angularDrag = 0.05f;
	float restitution = 0.3f; // bounciness [0..1]
	float friction = 0.5f;

	BodyType type = BodyType::Dynamic;
	bool useGravity = true;
	bool isSleeping = false;
	bool canSleep = true;

	// Apply an instantaneous impulse (mass-independent force)
	void AddImpulse(Vector3 impulse)
	{
		if (type == BodyType::Static) return;
		velocity = Vector3Add(velocity, Vector3Scale(impulse, 1.0f / mass));
		isSleeping = false;
	}

	// Apply a continuous force (integrated over deltaTime in PhysicsSystem)
	void AddForce(Vector3 force)
	{
		if (type == BodyType::Static) return;
		acceleration = Vector3Add(acceleration, Vector3Scale(force, 1.0f / mass));
		isSleeping = false;
	}

	float InverseMass() const
	{
		return (mass > 0.0f && type != BodyType::Static) ? 1.0f / mass : 0.0f;
	}
};