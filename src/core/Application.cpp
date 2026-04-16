#include "Application.h"
#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

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

	// Initialize ImGui for raylib
	rlImGuiSetup(true);

	// Pass registry into systems that need to query components
	m_render = RenderSystem(&m_registry, &m_assets);
	m_physics = PhysicsSystem(&m_registry);
	m_input = InputSystem();
	m_audio = AudioSystem(&m_registry, &m_assets);
}

void Application::Shutdown()
{
	m_assets.UnloadAll();
	rlImGuiShutdown();
	CloseAudioDevice();
	CloseWindow();
}

void Application::Tick(float deltaTime)
{
	if (IsKeyPressed(KEY_F3))
	{
		m_showDebugTools = !m_showDebugTools;

		if (m_showDebugTools)
		{
			m_cursorWasHidden = IsCursorHidden();
			EnableCursor();
		}
		else
		{
			if (m_cursorWasHidden) DisableCursor();
		}
	}

	if (IsKeyPressed(KEY_F4)) m_render.showDebug = !m_render.showDebug;

	// Fixed system order - PreUpdate -> Update -> Render
	m_input.Update();
	m_sceneManager.Update(deltaTime, m_registry, m_assets); // handles transitions
	m_physics.Update(deltaTime);
	OnUpdate(deltaTime);
	m_audio.Update();

	BeginDrawing();
	ClearBackground(BLACK);
	m_render.Update(deltaTime);

	if (m_showDebugTools)
	{
		rlImGuiBegin();

		ImGui::SetNextWindowPos({ 10,10 }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowBgAlpha(0.6f); // make the window slightly transparent

		if (ImGui::Begin("Engine Performance", nullptr, ImGuiViewportFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiViewportFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
		{
			ImGui::Text("FPS: %i", GetFPS());
			ImGui::Text("Frame Time %.3f ms", deltaTime * 1000.f);
			ImGui::Separator();
			ImGui::Text("Active Entities: %u / %u", m_registry.AliveCount(), Entity::MAX_ENTITIES);
			ImGui::Separator();
			ImGui::Checkbox("Show Colliders(F4)", &m_render.showDebug);
		}
		ImGui::End();

		DrawEntityInspector();

		rlImGuiEnd();
	}

	EndDrawing();
}

void Application::DrawEntityInspector()
{
	ImGui::Begin("Entity Inspector");

	// Left Pane: Entity list
	ImGui::BeginChild("EntityList", ImVec2(150, 0), true);
	ImGui::Text("Active Entities");
	ImGui::Separator();

	// Iterate over all entites that have a Spatial Component

	for (Entity e : m_registry.Each<Spatial>().GetEntities())
	{
		std::string label = "Entity " + std::to_string(e.Index());

		if (ImGui::Selectable(label.c_str(), m_selectedEntity == e))
		{
			m_selectedEntity = e; // select the entity when clicked
		}
	}

	ImGui::EndChild();

	ImGui::SameLine();

	// Right Pane: Component Details
	ImGui::BeginChild("ComponentDetails", ImVec2(0, 0), true);
	if (m_selectedEntity != Entity::Null && m_registry.IsAlive(m_selectedEntity))
	{
		ImGui::Text("Editing Entity ID: %u", m_selectedEntity.Index());
		ImGui::Separator();

		// SPATIAL COMPONENT
		if (m_registry.Has<Spatial>(m_selectedEntity))
		{
			if (ImGui::CollapsingHeader("Spatial", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto& spatial = m_registry.Get<Spatial>(m_selectedEntity);
				ImGui::DragFloat3("Position", &spatial.position.x, 0.1f);
				ImGui::DragFloat3("Rotation", &spatial.rotation.x, 1.0f);
				ImGui::DragFloat3("Scale", &spatial.scale.x, 0.05f);
			}
		}

		// RIGIDBODY COMPONENT
		if (m_registry.Has<Rigidbody>(m_selectedEntity))
		{
			if (ImGui::CollapsingHeader("Rigidbody", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto& rb = m_registry.Get<Rigidbody>(m_selectedEntity);
				ImGui::DragFloat("Mass", &rb.mass, 0.1f, 0.0f, 1000.0f);
				ImGui::DragFloat("Drag", &rb.drag, 0.05f, 0.0f, 10.0f);
				ImGui::DragFloat("Restitution", &rb.restitution, 0.05f, 0.0f, 1.0f);
				ImGui::Checkbox("Use Gravity", &rb.useGravity);
				ImGui::Checkbox("Can Sleep", &rb.canSleep);

				ImGui::BeginDisabled(); // Make velocity read-only
				ImGui::DragFloat3("Velocity", &rb.velocity.x);
				ImGui::EndDisabled();
			}
		}

		// BOX COLLIDER COMPONENT
		if (m_registry.Has<BoxCollider>(m_selectedEntity))
		{
			if (ImGui::CollapsingHeader("Box Collider", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto& col = m_registry.Get<BoxCollider>(m_selectedEntity);
				ImGui::DragFloat("Width", &col.width, 1.0f);
				ImGui::DragFloat("Height", &col.height, 1.0f);
				ImGui::DragFloat2("Offset", &col.offset.x, 1.0f);
				ImGui::Checkbox("Is Trigger", &col.isTrigger);
			}
		}

		// CAMERA 3D COMPONENT
		if (m_registry.Has<Camera3DComponent>(m_selectedEntity))
		{
			if (ImGui::CollapsingHeader("Camera 3D", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto& cam = m_registry.Get<Camera3DComponent>(m_selectedEntity);
				ImGui::DragFloat("FOV", &cam.fovy, 1.0f, 10.0f, 120.0f);
				ImGui::DragFloat("Sensitivity", &cam.sensitivity, 0.01f, 0.01f, 1.0f);
				ImGui::Checkbox("Invert X", &cam.invertX);
				ImGui::Checkbox("Invert Y", &cam.invertY);
			}
		}
	}
	else
	{
		ImGui::TextDisabled("Select an entity to view components.");
	}
	ImGui::EndChild();

	ImGui::End();
}