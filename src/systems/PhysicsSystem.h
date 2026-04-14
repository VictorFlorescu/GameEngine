#pragma once
#include "../ecs/Registry.h"
#include "../components/Components.h"
#include "raymath.h"

class PhysicsSystem
{
public:

	PhysicsSystem() = default;
	explicit PhysicsSystem(Registry* registry) : m_registry(registry) {}

	// Global gravity applied to all dynamic bodies with useGravity = true

	Vector3 gravity = { 0.0f, -9.81f, 0.0f };

	void Update(float deltaTime)
	{
		if (!m_registry) return;

		m_registry->View<Spatial, Rigidbody>([&](Entity e, Spatial& t, Rigidbody& rb)
			{
				if (rb.type == BodyType::Static) return;
				if (rb.isSleeping) return;

				// Apply gravity
				if (rb.useGravity)
					rb.acceleration = Vector3Add(rb.acceleration, gravity);

				// Integrate velocity
				rb.velocity = Vector3Add(rb.velocity, Vector3Scale(rb.acceleration, deltaTime));

				// Apply linear drag
				rb.velocity = Vector3Scale(rb.velocity, 1.0f - (rb.drag * deltaTime));

				// Integrate position
				t.position = Vector3Add(t.position, Vector3Scale(rb.velocity, deltaTime));

				// Integrate angular velocity into rotation
				t.rotation = Vector3Add(t.rotation, Vector3Scale(rb.angularVelocity, deltaTime));

				// Apply angular drag
				rb.angularVelocity = Vector3Scale(rb.angularVelocity, 1.0f - (rb.angularDrag * deltaTime));

				// Reset per-frame accumulated forces so AddForce() doesn't stack across frames
				rb.acceleration = { 0,0,0 };

				// Sleep if nearly still - avoids wasting update time on resting bodies

				float speedSq = Vector3LengthSqr(rb.velocity);
				if (speedSq < m_sleepThresholdSq && rb.type == BodyType::Dynamic)
					rb.isSleeping = true;

			});
	}

	void SetSleepThreshold(float speed) { m_sleepThresholdSq = speed * speed; }

private:
	Registry* m_registry = nullptr;
	float m_sleepThresholdSq = 0.001f * 0.001f;
};