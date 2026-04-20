#pragma once
#include "../ecs/Registry.h"
#include "../components/Components.h"

class AnimationSystem
{
public:
	AnimationSystem() = default;
	explicit AnimationSystem(Registry* registry) : m_registry(registry) {}

	void Update(float deltaTime)
	{
		if (!m_registry) return;

		m_registry->View<Animator2D, Sprite>([&](Entity e, Animator2D& anim, Sprite& sprite)
			{
				if (!anim.isPlaying || anim.totalFrames <= 1) return;

				// tick the timer
				anim.timer += deltaTime;

				// time for next frame?
				if (anim.timer >= anim.frameTime)
				{
					anim.timer -= anim.frameTime; // reset timer but keep remainder for smooth timing;
					anim.currentFrame++;

					// handle looping or stopping
					if (anim.currentFrame >= anim.totalFrames)
					{
						if (anim.loop) anim.currentFrame = 0;
						else
						{
							anim.currentFrame = anim.totalFrames - 1;
							anim.isPlaying = false;
						}
					}

				}

				//calculate the sprite's new source rectangle based on the grid
				if (sprite.texture.id != 0)
				{
					float frameWidth = (float)sprite.texture.width / anim.columns;
					float frameHeight = (float)sprite.texture.height / anim.rows;

					// math to convert an index, into a x,y position
					int currentColumn = anim.currentFrame % anim.columns;
					int currentRow = anim.currentFrame / anim.columns;

					sprite.srcRect =
					{
						currentColumn * frameWidth,
						currentRow * frameHeight,
						frameWidth,
						frameHeight
					};
				}
			});
	}
private:
	Registry* m_registry = nullptr;
};