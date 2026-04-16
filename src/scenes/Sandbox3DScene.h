#pragma once
#include "../scene/SceneManager.h"
#include "../ecs/Registry.h"
#include "../assets/AssetManager.h"
#include "../components/Components.h"
#include "raylib.h"
#include "raymath.h"

class Sandbox3DScene : public Scene
{
public:
	Entity cameraEntity;

	void OnEnter(Registry& registry, AssetManager& assets) override
	{
		DisableCursor();
		cameraEntity = registry.Create();

		auto& spatial = registry.Emplace<Spatial>(cameraEntity);
		spatial.position = { 0.0f,2.0f,10.0f };

		auto& cam = registry.Emplace<Camera3DComponent>(cameraEntity);
		cam.target = { 0.0f, 2.0f, 0.0f };
		cam.fovy = 60.0f;

		// create a cube to look at
		Entity cube = registry.Create();
		auto& cubeSpatial = registry.Emplace<Spatial>(cube);
		cubeSpatial.position = { 0.0f, 1.0f, 0.0f };
		cubeSpatial.scale = { 2.0f, 2.0f, 2.0f };

		auto& cubeMesh = registry.Emplace<MeshRenderer>(cube);
		cubeMesh = MeshRenderer::FromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
		cubeMesh.tint = RED;
	}

	void OnExit(Registry& registry, AssetManager& assets) override
	{
		EnableCursor(); //gimme my mouse back
	}

	void Update(float deltaTime, Registry& registry, AssetManager& assets) override
	{
		auto& t = registry.Get<Spatial>(cameraEntity);
		auto& cam = registry.Get<Camera3DComponent>(cameraEntity);

		// basic movement, not using InputSystem

		// mouse look
		if (IsCursorHidden())
		{
			Vector2 mouseDelta = GetMouseDelta();

			float yawDir = cam.invertX ? -1.0f : 1.0f;
			float pitchDir = cam.invertY ? -1.0f : 1.0f;

			t.rotation.y += mouseDelta.x * cam.sensitivity * yawDir;
			t.rotation.x += mouseDelta.y * cam.sensitivity * pitchDir;

			// clamp pitch so no neck breaking stuff
			if (t.rotation.x > 89.0f) t.rotation.x = 89.0f;
			if (t.rotation.x < -89.0f) t.rotation.x = -89.0f;

			// WASD movement

			float moveSpeed = 15.0f * deltaTime;

			Vector3 forward = t.Forward();
			Vector3 right = t.Right();

			if (IsKeyDown(KEY_W)) t.position = Vector3Add(t.position, Vector3Scale(forward, moveSpeed));
			if (IsKeyDown(KEY_S)) t.position = Vector3Subtract(t.position, Vector3Scale(forward, moveSpeed));
			if (IsKeyDown(KEY_D)) t.position = Vector3Add(t.position, Vector3Scale(right, moveSpeed));
			if (IsKeyDown(KEY_A)) t.position = Vector3Subtract(t.position, Vector3Scale(right, moveSpeed));

			// up and down

			if (IsKeyDown(KEY_SPACE)) t.position.y += moveSpeed;
			if (IsKeyDown(KEY_LEFT_SHIFT)) t.position.y -= moveSpeed;

		}

		// Update Camera Target

		cam.target = Vector3Add(t.position, t.Forward());

		// UI pass
		DrawText("3D SANDBOX", 20, 20, 30, RAYWHITE);
		DrawText("Mouse: Look around", 20, 70, 20, LIGHTGRAY);
		DrawText("W A S D: Move", 20, 100, 20, LIGHTGRAY);
		DrawText("SPACE / SHIFT: Up / Down", 20, 130, 20, LIGHTGRAY);
		DrawText("ESC: Unlock Mouse to Quit", 20, 160, 20, YELLOW);
	}
};