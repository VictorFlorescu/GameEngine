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

		// create player entity
		player = registry.Create();

		// Setup Spatial (pos and scale)
		auto& spatial = registry.Emplace<Spatial>(player);
		spatial.position = { 100.0f, 100.0f, 0.0f };
		spatial.scale = { 1.0f, 1.0f, 1.0f };

		// Setup Sprite
		auto& sprite = registry.Emplace<Sprite>(player);
		sprite.texture = assets.LoadTexture(fullPath);
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