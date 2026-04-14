#include "Application.h"
#include "raylib.h"

Application::Application(AppConfig config) : m_config(config) {}
Application::~Application() {}

void Application::Run()
{
	Init();
	OnStart();
	m_running = true;

	while (m_running && !WindowShouldClose())
	{
		float deltaTime = GetFrameTime();
		Tick(deltaTime);
	}

	OnStop();
	Shutdown();
}

void Application::Init()
{
	InitWindow(m_config.width, m_config.height, m_config.title);
	SetTargetFPS(m_config.fps);
	InitAudioDevice();

	// Pass registry into systems that need to query components
	m_render = RenderSystem(&m_registry, &m_assets);
	m_physics = PhysicsSystem(&m_registry);
	m_input = InputSystem();
	m_audio = AudioSystem(&m_registry, &m_assets);
}

void Application::Shutdown()
{
	m_assets.UnloadAll();
	CloseAudioDevice();
	CloseWindow();
}

void Application::Tick(float deltaTime)
{
	// Fixed system order - PreUpdate -> Update -> Render
	m_input.Update();
	m_sceneManager.Update(deltaTime, m_registry, m_assets); // handles transitions
	m_physics.Update(deltaTime);
	OnUpdate(deltaTime);
	m_audio.Update();

	BeginDrawing();
	ClearBackground(BLACK);
	m_render.Update(deltaTime);
	EndDrawing();
}