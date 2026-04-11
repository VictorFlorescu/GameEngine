#include "EntityManager.h"

EntityManager::EntityManager(uint32_t maxEntities) : m_maxEntities(maxEntities), m_generations(maxEntities, 0)
{
	// Initially all indices are free
	for (uint32_t i = 0; i < maxEntities; ++i)
		m_freeList.push(i);
}

Entity EntityManager::Create()
{
	assert(!m_freeList.empty() && "Entity limit reached");

	uint32_t index = m_freeList.front();
	m_freeList.pop();
	++m_aliveCount;

	return Entity::Make(index, m_generations[index]);
}

void EntityManager::Destroy(Entity entity)
{
	assert(IsAlive(entity) && "Destroying a dead or stale entity");

	uint32_t index = entity.Index();

	// Bump the generation - any existing handles with the old gen are now stale
	++m_generations[index];
	m_freeList.push(index);
	--m_aliveCount;
}

bool EntityManager::IsAlive(Entity entity) const
{
	if (entity == Entity::Null) return false;
	uint32_t index = entity.Index();
	if (index >= m_maxEntities) return false;
	return m_generations[index] == entity.Generation();
}