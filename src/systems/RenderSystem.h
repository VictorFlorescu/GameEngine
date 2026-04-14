#pragma once
#include "../ecs/Registry.h"
#include "../assets/AssetManager.h"
#include "../components/Components.h"
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <algorithm>

class RenderSystem
{
public:
	RenderSystem() = default;
	RenderSystem(Registry* registry, AssetManager* assets) : m_registry(registry), m_assets(assets) {}

	// Call once at startup if you need a 3D camera
	void SetCamera3D(Camera3D cam) { m_camera3D = cam; m_has3D = true; }
	Camera3D& GetCamera3D() { return m_camera3D; }

	// Call once at startup if you need a 2D camera (pan/zoom)
	void SetCamera2D(Camera2D cam) { m_camera2D = cam; m_has2D = true; }
	Camera2D& GetCamera2D() { return m_camera2D; }

	void Update(float deltaTime)
	{
		Draw3D();
		Draw2D();
		DrawUI(); // always drawn last, no camera
	}

private:
	Registry* m_registry = nullptr;
	AssetManager* m_assets = nullptr;

	Camera3D m_camera3D = {};
	Camera2D m_camera2D = {};
	bool m_has3D = false;
	bool m_has2D = false;

	
	// 3D pass

	void Draw3D()
	{
		if (!m_has3D || !m_registry) return;

		BeginMode3D(m_camera3D);

		m_registry->View<Spatial, MeshRenderer>([](Entity e, Spatial& t, MeshRenderer& mr)
		{
			if (!mr.visible) return;

			// Build a transform matrix from position/rotation/scale
			// and draw the model using it directly

			DrawModelEx(
				mr.model,
				t.position,
				{ 0.f, 1.f, 0.f }, // rotation axis (Y up)
				t.rotation.y, // primary rotation angle
				t.scale,
				mr.tint
			);
		});

		EndMode3D();
	}

	// 2D pass - sprites sorted by layer

	struct SpriteDrawCmd
	{
		Spatial* transform;
		Sprite* sprite;
		int layer;
	};

	void Draw2D()
	{
		// Collect all visible sprites into draw commands
		std::vector<SpriteDrawCmd> cmds;
		cmds.reserve(128);

		m_registry->View<Spatial, Sprite>([&](Entity e, Spatial& t, Sprite s)
			{
				if (!s.visible) return;
				cmds.push_back({ &t, &s, s.layer });
			});

		// Sort by layer - stable_sort preserves submittion order within a layer
		std::stable_sort(cmds.begin(), cmds.end(), [](const SpriteDrawCmd& a, const SpriteDrawCmd& b)
			{
				return a.layer < b.layer;
			});

		if (m_has2D) BeginMode2D(m_camera2D);

		for (auto& cmd : cmds)
		{
			Spatial& t = *cmd.transform;
			Sprite& s = *cmd.sprite;

			Rectangle src = s.GetSrcRect();
			if (s.flipX) src.width = -src.width;
			if (s.flipY) src.height = -src.height;

			Rectangle dst = {
				t.position.x,
				t.position.y,
				src.width * t.scale.x,
				src.height * t.scale.y
			};

			Vector2 origin = s.GetOriginPixels();
			origin.x *= t.scale.x;
			origin.y *= t.scale.y;

			DrawTexturePro(s.texture, src, dst, origin, t.rotation.z, s.tint);
		}

		if (m_has2D) EndMode2D();
	}
	 // UI pass
	void DrawUI()
	{
		//PLACEHOLDER
	}
};