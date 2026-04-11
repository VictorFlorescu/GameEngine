#pragma once
#include "ecs/Registry.h"
#include "scene/SceneManager.h"
#include "assets/AssetManager.h"
#include "systems/RenderSystem.h"
#include "systems/InputSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/AudioSystem.h"

struct AppConfig
{
	const char* title = "Game";
	int width = 1280;
	int height = 720;
	int fps = 60;
};

class Application
{
public:
	explicit Application(AppConfig config = {});
	virtual ~Application();

	// Call this from main() - blocks until the window closes
	void Run();

	Registry& GetRegistry() { return m_registry; }
	SceneManager& GetSceneManager() { return m_sceneManager; }
	AssetManager& GetAssets() { return m_assets; }
	InputSystem& GetInput() { return m_input; }

protected:
	virtual void OnStart() {}
	virtual void OnUpdate(float deltaTime) {}
	virtual void OnStop() {}

private:
	void Init();
	void Shutdown();
	void Tick(float deltaTime);

	AppConfig m_config;
	Registry m_registry;
	AssetManager m_assets;
	SceneManager m_sceneManager;
	RenderSystem m_render;
	PhysicsSystem m_physics;
	InputSystem m_input;
	AudioSystem m_audio;
	bool m_running = false;
};