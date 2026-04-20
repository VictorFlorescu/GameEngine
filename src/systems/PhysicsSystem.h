#pragma once
#include "../ecs/Registry.h"
#include "../components/Components.h"
#include "raymath.h"
#include <vector>
#include <algorithm>



class PhysicsSystem
{
public:

	PhysicsSystem() = default;
	explicit PhysicsSystem(Registry* registry) : m_registry(registry) {}

	// Global gravity applied to all dynamic bodies with useGravity = true

	Vector3 gravity = { 0.0f, 981.0f, 0.0f };

	void Update(float deltaTime)
	{
		if (!m_registry) return;

		m_registry->View<Spatial, Rigidbody>([&](Entity e, Spatial& t, Rigidbody& rb)
			{
				if (rb.type == BodyType::Static) return;

				// Apply gravity
				if (rb.useGravity)
					rb.acceleration = Vector3Add(rb.acceleration, gravity);

				// Integrate velocity
				rb.velocity = Vector3Add(rb.velocity, Vector3Scale(rb.acceleration, deltaTime));

				// Apply linear drag
				float dragFactor = expf(-rb.drag * deltaTime);
				rb.velocity = Vector3Scale(rb.velocity, dragFactor);

				// Integrate position
				t.position = Vector3Add(t.position, Vector3Scale(rb.velocity, deltaTime));

				// Integrate angular velocity into rotation
				t.rotation = Vector3Add(t.rotation, Vector3Scale(rb.angularVelocity, deltaTime));

				// Apply angular drag
				float angDragFactor = expf(-rb.angularDrag * deltaTime);
				rb.angularVelocity = Vector3Scale(rb.angularVelocity, angDragFactor);

				// Reset per-frame accumulated forces so AddForce() doesn't stack across frames
				rb.acceleration = { 0,0,0 };

				// Sleep if nearly still - avoids wasting update time on resting bodies

				if (rb.canSleep && rb.type == BodyType::Dynamic)
				{
					float speedSq = Vector3LengthSqr(rb.velocity);
					if (speedSq < m_sleepThresholdSq)
						rb.isSleeping = true;
				}

			});

		std::vector<PhysNode> nodes;

		m_registry->View<Spatial, Rigidbody, BoxCollider>([&](Entity e, Spatial& t, Rigidbody& rb, BoxCollider& col)
			{
				nodes.push_back({ e,&t,&rb,&col });
			});
		// pairwise checks
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			for (size_t j = i + 1; j < nodes.size(); ++j)
			{
				ResolveAABB(nodes[i], nodes[j]);
			}
		}
	}

	void SetSleepThreshold(float speed) { m_sleepThresholdSq = speed * speed; }

private:
	Registry* m_registry = nullptr;
	float m_sleepThresholdSq = 0.001f * 0.001f;

	struct PhysNode { Entity  e; Spatial* t; Rigidbody* rb; BoxCollider* col; };

	void ResolveAABB(PhysNode& a, PhysNode& b)
	{
		// Don't calculate collisions between static objects
		if (a.rb->type == BodyType::Static && b.rb->type == BodyType::Static)  return;

		// Calculate centers 
		// TODO: convert this and anything else that is required for this to work for 3D as well
		Vector3 centerA = { a.t->position.x + a.col->offset.x, a.t->position.y + a.col->offset.y, /*z?*/ 0.0f };
		Vector3 centerB = { b.t->position.x + b.col->offset.x, b.t->position.y + b.col->offset.y, /*z?*/ 0.0f };

		// Calculate half-extents (accounting for Spatial scale)
		Vector2 halfA = { (a.col->width / 2.0f) * a.t->scale.x, (a.col->height / 2.0f) * a.t->scale.y };
		Vector2 halfB = { (b.col->width / 2.0f) * b.t->scale.x, (b.col->height / 2.0f) * b.t->scale.y };

		Vector3 delta = Vector3Subtract(centerB, centerA);

		// Check for overlap on X axis
		float overlapX = (halfA.x + halfB.x) - fabsf(delta.x);
		if (overlapX > 0)
		{
			// Check for overlap on Y axis
			float overlapY = (halfA.y + halfB.y) - fabsf(delta.y);
			if (overlapY > 0)
			{
				// collision confirmed
				// find the axis of least penetration to push them out correctly

				if (a.col->onCollision) a.col->onCollision(a.e, b.e);
				if (b.col->onCollision) b.col->onCollision(b.e, a.e);

				// if either object  is a trigger, stop here
				if (a.col->isTrigger || b.col->isTrigger) return;
				

				Vector3 normal = { 0,0,0 };
				float penetration = 0;

				if (overlapX < overlapY)
				{
					normal.x = delta.x < 0 ? -1.0f : 1.0f;
					penetration = overlapX;
				}
				else
				{
					normal.y = delta.y < 0 ? -1.0f : 1.0f;
					penetration = overlapY;
				}

				float invA = a.rb->InverseMass();
				float invB = b.rb->InverseMass();
				float sumMass = invA + invB;
				if (sumMass == 0) return;

				// Positional correction (push apart so they don't sink)
				// Multiply by 0.8f to prevent jittering (stop allowance)
				Vector3 correction = Vector3Scale(normal, (penetration / sumMass) * 0.8f);
				a.t->position = Vector3Subtract(a.t->position, Vector3Scale(correction, invA));
				b.t->position = Vector3Add(b.t->position, Vector3Scale(correction, invB));

				// Velocity resolution (bounce)
				Vector3 relVel = Vector3Subtract(b.rb->velocity, a.rb->velocity);
				float velAlongNormal = Vector3DotProduct(relVel, normal);

				// if they are alreadt moving apart, dont bounce them again
				if (velAlongNormal > 0) return;

				// Calculate restiturion/bounciness - take the lowest of the two
				float e = std::min(a.rb->restitution, b.rb->restitution);

				// calculate Impulse scalar
				float j = -(1.0f + e) * velAlongNormal;
				j /= sumMass;

				// apply impulse vectors to velocities
				Vector3 impulse = Vector3Scale(normal, j);
				a.rb->velocity = Vector3Subtract(a.rb->velocity, Vector3Scale(impulse, invA));
				b.rb->velocity = Vector3Add(b.rb->velocity, Vector3Scale(impulse, invB));

				a.rb->isSleeping = false;
				b.rb->isSleeping = false;
			}
		}
	}
};