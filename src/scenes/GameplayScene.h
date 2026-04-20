#pragma once
#include "../scene/SceneManager.h"
#include "../ecs/Registry.h"
#include "../assets/AssetManager.h"
#include "../components/Components.h"
#include "raylib.h"

class GameplayScene : public Scene
{
public:
	Entity player;
	void OnEnter(Registry& registry, AssetManager& assets) override
	{
		std::string resPath = RESOURCES_PATH;
		std::string fullPath = resPath + "demo_texture.png";
		std::string playerAnimPath = resPath + "playerAnim.png";

		// create player entity
		player = registry.Create();

		auto& anim = registry.Emplace<Animator2D>(player);
		anim.loop = true;
		anim.columns = 8;
		anim.rows = 1;
		anim.totalFrames = anim.columns * anim.rows;
		anim.frameTime = 0.3f;

		// Setup Spatial (pos and scale)
		auto& spatial = registry.Emplace<Spatial>(player);
		spatial.position = { 100.0f, 100.0f, 0.0f };
		spatial.scale = { 1.0f, 1.0f, 1.0f };

		// Setup Sprite
		auto& sprite = registry.Emplace<Sprite>(player);
		sprite.texture = assets.LoadTexture(playerAnimPath);
		sprite.origin = { 0.5f, 0.5f };

		// Setup RigidBody
		auto& rb = registry.Emplace<Rigidbody>(player);
		rb.type = BodyType::Dynamic;
		rb.useGravity = false;
		rb.mass = 2.0f;
		rb.drag = 1.5f;
		rb.canSleep = false;
		rb.restitution = 1.0f;

		registry.Emplace<BoxCollider>(player);

		auto& cam = registry.Emplace<Camera2DComponent>(player);
		cam.zoom = 1.0f;

		// create a static wall

		Entity wall = registry.Create();

		auto& wSpatial = registry.Emplace<Spatial>(wall);
		wSpatial.position = { 100.0f, 400.0f, 0.0f }; // Place on the right
		wSpatial.scale = { 12.0f, 1.0f, 1.0f }; // Make it a tall rectangle

		auto& wSprite = registry.Emplace<Sprite>(wall);
		wSprite.texture = assets.LoadTexture(fullPath);
		wSprite.origin = { 0.5f, 0.5f };
		wSprite.tint = BLUE; // Make the wall blue

		auto& wRb = registry.Emplace<Rigidbody>(wall);
		wRb.type = BodyType::Static; // CRITICAL: Static bodies have infinite mass!
		wRb.restitution = 1.f;

		// ADD THE COLLIDER
		registry.Emplace<BoxCollider>(wall);

		// --- 3. CREATE A TRIGGER ZONE (e.g., A Coin) ---
		Entity coin = registry.Create();

		auto& cSpatial = registry.Emplace<Spatial>(coin);
		cSpatial.position = { 500.0f, 360.0f, 0.0f }; // Place it between the player and the wall
		cSpatial.scale = { 1.0f, 1.0f, 1.0f };

		auto& cSprite = registry.Emplace<Sprite>(coin);
		cSprite.texture = assets.LoadTexture(fullPath);
		cSprite.tint = YELLOW; // Make it yellow so we know it's a pickup
		cSprite.origin = { 0.5f, 0.5f };

		// Triggers don't necessarily need Rigidbodies if they are static, 
		// but the physics system requires both right now to check collisions.
		auto& cRb = registry.Emplace<Rigidbody>(coin);
		cRb.type = BodyType::Static;

		auto& cCol = registry.Emplace<BoxCollider>(coin);
		cCol.isTrigger = true; // IMPORTANT!

		// THE MAGIC: Write the gameplay logic directly into the component!
		// We capture the registry by reference [&registry] so we can modify the world.
		cCol.onCollision = [&registry](Entity me, Entity other)
			{
				// In a real game, you'd check if 'other' is the player. 
				// For now, if anything touches this coin, destroy it!
				if (registry.IsAlive(me))
				{
					printf("Coin collected by Entity %d!\n", other.Index());
					registry.Destroy(me);
				}
			};

		// --- TEST PREFABS & PARTICLES ---

	// 1. Spawn the entity using the JSON file!
		Entity firePit = PrefabManager::Load(resPath + "fire_pit.json", registry, assets);

		// 2. Attach a Particle Emitter and a Collider via C++ to complete it
		if (registry.IsAlive(firePit))
		{
			// Give it a Box Collider so we can see it with F4 (Debug Draw)
			auto& col = registry.Emplace<BoxCollider>(firePit);
			col.width = 40.0f;
			col.height = 40.0f;

			// Attach the Particle Emitter!
			auto& emitter = registry.Emplace<ParticleEmitter>(firePit);
			emitter.isEmitting = true;
			emitter.emitRate = 0.02f;         // Spit out a particle extremely fast
			emitter.startColor = ORANGE;      // Fire colors
			emitter.endColor = RED;
			emitter.speed = 80.0f;            // Shoot out fast
			emitter.particleLifetime = 0.8f;  // Burn out quickly
		}
	}

	void Update(float deltaTime, Registry& registry, AssetManager& assets) override
	{
		// Handle player input and physics
		if (registry.Has<Rigidbody>(player))
		{
			auto& rb = registry.Get<Rigidbody>(player);

			// AddForce applies gradual, continuous movement like thrusters
			if (IsKeyDown(KEY_W)) rb.AddForce({ 0.0f, -2000.0f, 0.0f });
			if (IsKeyDown(KEY_S)) rb.AddForce({ 0.0f, 2000.0f, 0.0f });
			if (IsKeyDown(KEY_A)) rb.AddForce({ -2000.0f, 0.0f, 0.0f });
			if (IsKeyDown(KEY_D)) rb.AddForce({ 2000.0f, 0.0f, 0.0f });
			
			if (IsKeyPressed(KEY_SPACE))
			{
				// jump
				rb.AddImpulse({ 0.0f, -1500.0f, 0.0f });
			}

			
		}

		// Draw ui instructions
		DrawText("TOP-DOWN ENGINE DEMO", 20, 20, 30, RAYWHITE);
		DrawText("W A S D - Apply continuous Force (Thrusters)", 20, 70, 20, LIGHTGRAY);
		DrawText("SPACE   - Apply instant Impulse (Dash Right)", 20, 100, 20, LIGHTGRAY);
		DrawText("Notice how the Rigidbody drag slows you down naturally!", 20, 140, 20, GREEN);
	}
};