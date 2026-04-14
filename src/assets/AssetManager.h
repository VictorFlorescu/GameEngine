#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>
#include <stdexcept>

// Type tags - used to key separate caches per asset type
struct TextureAsset { using Type = Texture2D; };
struct SoundAsset { using Type = Sound; };
struct MusicAsset { using Type = Music; };
struct FontAsset { using Type = Font; };
struct ModelAsset { using Type = Model; };
struct ShaderAsset { using Type = Shader; };

class AssetManager
{
public:

	// Load - loads from disk on first call, returns cached on repeat

	Texture2D& LoadTexture(const std::string& path)
	{
		return LoadOrGet(m_textures, path, [](const std::string& p) 
			{
			Texture2D t = ::LoadTexture(p.c_str());
			SetTextureFilter(t, TEXTURE_FILTER_BILINEAR);
			return t;
			});
	}

	Sound& LoadSound(const std::string& path)
	{
		return LoadOrGet(m_sounds, path, [](const std::string& p) 
			{
			return ::LoadSound(p.c_str());
			});
	}

	Font& LoadFont(const std::string& path, int size = 32)
	{
		// Key includes size so the same font at different sizes can be cached separatedly
		std::string key = path + "@" + std::to_string(size);
		return LoadOrGet(m_fonts, key, [&](const std::string&) 
			{
			return ::LoadFontEx(path.c_str(), size, nullptr, 0);
			});
	}
	Model& LoadModel(const std::string& path)
	{
		return LoadOrGet(m_models, path, [this](const std::string& p) 
			{
			return LoadModel(p.c_str());
			});
	}

	Shader& LoadShader(const std::string& vsPath, const std::string& fsPath)
	{
		std::string key = vsPath + "|" + fsPath;
		return LoadOrGet(m_shaders, key, [&](const std::string&)
			{
				const char* vs = vsPath.empty() ? nullptr : vsPath.c_str();
				const char* fs = fsPath.empty() ? nullptr : fsPath.c_str();
				return ::LoadShader(vs, fs);
			});
	}

	// Get - retrieves a previously loaded asset, asserts if missing

	Texture2D& GetTexture(const std::string& path) { return Get(m_textures, path); }
	Sound& GetSound(const std::string& path) { return Get(m_sounds, path); }
	Music& GetMusic(const std::string& path) { return Get(m_music, path); }
	Model& GetModel(const std::string& path) { return Get(m_models, path); }
	Font& GetFont(const std::string& path, int size = 32)
	{
		return Get(m_fonts, path + "@" + std::to_string(size));
	}

	// Unload - explicit unload of one asset

	void UnloadTexture(const std::string& path)
	{
		Unload(m_textures, path, [](Texture2D& t) { ::UnloadTexture(t);});
	}
	void UnloadSound(const std::string& path)
	{
		Unload(m_sounds, path, [](Sound& s) { ::UnloadSound(s);});
	}
	void UnloadModel(const std::string& path)
	{
		Unload(m_models, path, [](Model& m) { ::UnloadModel(m);});
	}

	// UnloadAll - call this in Application::Shutdown
	void UnloadAll()
	{
		for (auto& [k, t] : m_textures) ::UnloadTexture(t);
		for (auto& [k, s] : m_sounds) ::UnloadSound(s);
		for (auto& [k, m] : m_music) ::UnloadMusicStream(m);
		for (auto& [k, f] : m_fonts) ::UnloadFont(f);
		for (auto& [k, m] : m_models) ::UnloadModel(m);
		for (auto& [k, s] : m_shaders) ::UnloadShader(s);

		m_textures.clear();
		m_sounds.clear();
		m_music.clear();
		m_fonts.clear();
		m_models.clear();
		m_shaders.clear();
	}

	// Exists - check without asserting
	bool HasTexture(const std::string& path) const { return m_textures.count(path) > 0; }
	bool HasSound(const std::string& path) const { return m_sounds.count(path) > 0; }
	bool HasModel(const std::string& path) const { return m_models.count(path) > 0; }
	bool HasMusic(const std::string& path) const { return m_music.count(path) > 0; }

private:
	std::unordered_map<std::string, Texture2D> m_textures;
	std::unordered_map<std::string, Sound> m_sounds;
	std::unordered_map<std::string, Music> m_music;
	std::unordered_map<std::string, Font> m_fonts;
	std::unordered_map<std::string, Model> m_models;
	std::unordered_map<std::string, Shader> m_shaders;

	// Internals

	template<typename T, typename LoadFn>
	T& LoadOrGet(std::unordered_map<std::string, T>& cache, const std::string& key, LoadFn&& load)
	{
		auto it = cache.find(key);
		if (it != cache.end()) return it->second;
		cache[key] = load(key);
		return cache[key];
	}

	template<typename T>
	T& Get(std::unordered_map<std::string, T>& cache, const std::string& key)
	{
		auto it = cache.find(key);
		assert(it != cache.end() && "Asset not loaded - Call Load before Get");
		return it->second;
	}
	template<typename T, typename UnloadFn>
	void Unload(std::unordered_map<std::string, T>& cache, const std::string& key, UnloadFn&& unload)
	{
		auto it = cache.find(key);
		if (it == cache.end()) return;
		unload(it->second);
		cache.erase(it);
	}
};