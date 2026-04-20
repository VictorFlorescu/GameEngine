#pragma once
#include "../ecs/Registry.h"
#include "../components/Components.h"

class ParticleSystem
{
public:
	ParticleSystem() = default;
	explicit ParticleSystem(Registry* registry) : m_registry(registry) {}

	void Update(float deltaTime)
	{
		if (!m_registry) return;
		
		m_registry->View<Spatial, ParticleEmitter>([&](Entity e, Spatial& t, ParticleEmitter& emitter)
			{
				// Spawn logic
				if (emitter.isEmitting)
				{
					emitter.emitTimer += deltaTime;
					while (emitter.emitTimer >= emitter.emitRate)
					{
						emitter.emitTimer -= emitter.emitRate;
						Particle p;
						p.position = t.position; // spawn at entity's location

						// random spray direction
						float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
						p.velocity = { cosf(angle) * emitter.speed, sinf(angle) * emitter.speed, 0.0f };

						p.maxLife = emitter.particleLifetime;
						p.color = emitter.startColor;
						emitter.particles.push_back(p);
					}
				}

				// Update and draw loop
				for (int i = (int)emitter.particles.size() - 1; i >= 0; i--)
				{
					Particle& p = emitter.particles[i];
					p.life += deltaTime;

					if (p.life >= p.maxLife)
					{
						// kill particle
						emitter.particles.erase(emitter.particles.begin() + i);
						continue;
					}

					// move
					p.position.x += p.velocity.x * deltaTime;
					p.position.y += p.velocity.y * deltaTime;

					// fade color over time
					float lifePct = p.life / p.maxLife;
					p.color.r = Lerp(emitter.startColor.r, emitter.endColor.r, lifePct);
					p.color.g = Lerp(emitter.startColor.g, emitter.endColor.g, lifePct);
					p.color.b = Lerp(emitter.startColor.b, emitter.endColor.b, lifePct);
					p.color.a = Lerp(255, 0, lifePct); // Fade to transparent

				}
			});
	}

private:
	Registry* m_registry = nullptr;
};