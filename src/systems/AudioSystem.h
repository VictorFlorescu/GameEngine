#pragma once
#include "../ecs/Registry.h"
#include "../assets/AssetManager.h"
#include "../components/AudioSource.h"
#include "raylib.h"

class AudioSystem
{
public:
	AudioSystem() = default;
	AudioSystem(Registry* registry, AssetManager* assets) : m_registry(registry), m_assets(assets) {}

	void Update()
	{
		if (!m_registry || !m_assets) return;

		m_registry->View<AudioSource>([&](Entity e, AudioSource& src)
			{
				if (src.soundPath.empty()) return;

				// Autoplay on first frame if requested
				if (!src.m_started)
				{
					src.m_started = true;
					if (src.playOnStart) Play(src);
				}

				// Keep music streams updated - sounds are fire-and-forget
				if (src.loop && src.m_playing)
				{
					if (m_assets->HasMusic(src.soundPath))
					{
						Music& music = m_assets->GetMusic(src.soundPath);
						UpdateMusicStream(music);
					}
				}

			});
	}

	// Call these directly from game or scene logic

	void Play(AudioSource& src)
	{
		if (!m_assets) return;
		SetSoundVolume(m_assets->GetSound(src.soundPath), src.volume);
		SetSoundPitch(m_assets->GetSound(src.soundPath), src.pitch);
		PlaySound(m_assets->GetSound(src.soundPath));
		src.m_playing = true;
	}

	void PlayMusic(AudioSource& src)
	{
		if (!m_assets) return;
		Music& music = m_assets->GetMusic(src.soundPath);
		SetMusicVolume(music, src.volume);
		SetMusicPitch(music, src.pitch);
		PlayMusicStream(music);
		src.m_playing = true;
	}
	void Stop(AudioSource& src)
	{
		if (!m_assets) return;
		if (m_assets->HasSound(src.soundPath))
			StopSound(m_assets->GetSound(src.soundPath));
		if (m_assets->HasMusic(src.soundPath))
			StopMusicStream(m_assets->GetMusic(src.soundPath));
		src.m_playing = false;
	}

private:
	Registry* m_registry = nullptr;
	AssetManager* m_assets = nullptr;
};