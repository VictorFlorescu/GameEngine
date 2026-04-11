#pragma once
#include "Entity.h"
#include <vector>
#include <queue>
#include <cassert>

class EntityManager
{
public:
	explicit EntityManager(uint32_t maxEntities = Entity::MAX_ENTITIES);

	Entity Create();
	void Destroy(Entity entity);
	bool IsAlive(Entity entity) const;

	uint32_t AliveCount() const { return m_aliveCount; }

private:
	uint32_t m_maxEntities;
	uint32_t m_aliveCount = 0;

	// One generation counter per slot - bumped on every Destroy
	std::vector<uint32_t> m_generations;

	// Recycled indices waiting to be reused
	std::queue<uint32_t> m_freeList;
};