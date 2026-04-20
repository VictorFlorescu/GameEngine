#pragma once
#include "../ecs/Registry.h"
#include "../components/Components.h"
#include "AssetManager.h"
#include <fstream>
#include <json.h>

using json = nlohmann::json;

class PrefabManager
{
public:
	static Entity Load(const std::string& filepath, Registry& registry, AssetManager& assets)
	{
		std::ifstream file(filepath);
		if (!file.is_open()) return Entity::Null;

		json data;
		file >> data;

		Entity e = registry.Create();

		// SPATIAL COMPONENT
		if (data.contains("Spatial"))
		{
			auto& t = registry.Emplace<Spatial>(e);
			t.position = { data["Spatial"]["pos"][0], data["Spatial"]["pos"][1], data["Spatial"]["pos"][2] };
			t.scale = { data["Spatial"]["scale"][0], data["Spatial"]["scale"][1], data["Spatial"]["scale"][2] };
		}

		// SPRITE COMPONENT
		if (data.contains("Sprite"))
		{
			auto& s = registry.Emplace<Sprite>(e);
			std::string texPath = data["Sprite"]["texture"];
			s.texture = assets.LoadTexture(texPath.c_str());
		}

		// RIGIDBODY COMPONENT
		if (data.contains("Rigidbody"))
		{
			auto& rb = registry.Emplace<Rigidbody>(e);
			rb.mass = data["Rigidbody"].value("mass", 1.0f);
			std::string type = data["Rigidbody"].value("type", "Dynamic");
			rb.type = (type == "Static") ? BodyType::Static : BodyType::Dynamic;
		}

		return e;
	}
};