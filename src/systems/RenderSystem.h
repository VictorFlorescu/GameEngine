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
	explicit RenderSystem(Registry* registry, AssetManager* assets) : m_registry(registry), m_assets(assets) {}

	bool showDebug = false;

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
		if (!m_registry) return;


		Camera3D activeCamera = { 0 };
		bool useCamera = false;

		m_registry->View<Spatial, Camera3DComponent>([&](Entity e, Spatial& t, Camera3DComponent& cam)
			{
				if (cam.isMain)
				{
					activeCamera.position = t.position;
					activeCamera.target = cam.target;
					activeCamera.up = cam.up;
					activeCamera.fovy = cam.fovy;
					activeCamera.projection = cam.projection;
					useCamera = true;
				}
			});

		if (!useCamera) return; // skip pass if no 3D camera exists

		BeginMode3D(activeCamera);

		m_registry->View<Spatial, MeshRenderer>([](Entity e, Spatial& t, MeshRenderer& mr)
		{
			if (!mr.visible) return;

			mr.model.transform = t.GetMatrix();

			DrawModel(mr.model, { 0.0f, 0.0f,0.0f }, 1.0f, mr.tint);

		});

		DrawGrid(20, 10.0f);

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
		Camera2D activeCamera = { 0 };
		bool useCamera = false;

		m_registry->View<Spatial, Camera2DComponent>([&](Entity e, Spatial& t, Camera2DComponent& cam)
			{
				if (cam.isMain)
				{
					// map the ecs component data into raylib's camera
					activeCamera.target = { t.position.x, t.position.y };
					activeCamera.offset = cam.offset;
					activeCamera.rotation = t.rotation.z;
					activeCamera.zoom = cam.zoom;
					useCamera = true;
				}
			}
		);

		// Collect all visible sprites into draw commands
		std::vector<SpriteDrawCmd> cmds;
		cmds.reserve(128);

		m_registry->View<Spatial, Sprite>([&](Entity e, Spatial& t, Sprite& s)
			{
				if (!s.visible) return;
				cmds.push_back({ &t, &s, s.layer });
			});

		// Sort by layer - stable_sort preserves submittion order within a layer
		std::stable_sort(cmds.begin(), cmds.end(), [](const SpriteDrawCmd& a, const SpriteDrawCmd& b)
			{
				return a.layer < b.layer;
			});

		if (useCamera) BeginMode2D(activeCamera);

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

		if (showDebug)
		{
			// draw hitboxes
			m_registry->View<Spatial, BoxCollider>([&](Entity e, Spatial& t, BoxCollider& col)
				{
					float w = col.width * t.scale.x;
					float h = col.height * t.scale.y;
					float x = (t.position.x + col.offset.x) - (w / 2.0f);
					float y = (t.position.y + col.offset.y) - (h / 2.0f);

					// Triggers are yellow, solid colliders are green

					Color color = col.isTrigger ? YELLOW : GREEN;

					DrawRectangleLinesEx({ x,y,w,h }, 2.0f, color);

					// Draw a tiny crosshair at the center of mass
					DrawCircle(t.position.x, t.position.y, 2.0f, BLUE);
				});
			// draw velocity vectors
			m_registry->View<Spatial, Rigidbody>([&](Entity e, Spatial& t, Rigidbody& rb)
				{
					if (Vector3LengthSqr(rb.velocity) > 0.1f)
					{
						Vector2 start = { t.position.x, t.position.y };

						// multiply by 0.5f so a big speed doesnt draw a line of the screen
						Vector2 end =
						{
							t.position.x + (rb.velocity.x * 0.5f),
							t.position.y + (rb.velocity.y * 0.5f),
						};

						DrawLineEx(start, end, 2.0f, RED);
					}
				});
		}

		if (useCamera) EndMode2D();
	}
	 // UI pass
	void DrawUI()
	{
		//PLACEHOLDER
	}
};