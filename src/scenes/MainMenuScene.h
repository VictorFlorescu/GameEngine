#pragma once
#include "../scene/SceneManager.h"
#include "../ecs/Registry.h"
#include "../assets/AssetManager.h"
#include "raylib.h"

class MainMenuScene : public Scene
{
public:
	void OnEnter(Registry& registry, AssetManager& assets) override
	{
		// TODO: Load menu assets, create UI entities
	}

	void OnExit(Registry& registry, AssetManager& assets) override
	{
		// TODO: Unload menu assets, destroy UI entities
	}

	void Update(float deltaTime, Registry& registry, AssetManager& assets) override
	{
		// TODO: Draw menu, handle input

		DrawText("Main Menu", 100, 100, 40, WHITE);
		DrawText("Press ENTER to start", 100, 160, 20, LIGHTGRAY);
	}
};