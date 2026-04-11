#pragma once
#include "IComponentArray.h"
#include <vector>
#include <cassert>

template<typename T>
class ComponentArray : public IComponentArray
{
public:
	explicit ComponentArray(uint32_t maxEntities) : m_sparse(maxEntities, INVALID) {}

	// Add a component to an entity (forwards constructor args)
	template<typename... Args>
	T& Emplace(Entity entity, Args&&... args)
	{
		uint32_t idx = entity.Index();
		assert(m_sparse[idx] == INVALID && "Entity already has this component");

		// Record where in the dense array this entity lives
		m_sparse[idx] = static_cast<uint32_t>(m_dense.size());

		m_dense.push_back(T{ std::forward<Args>(args)... });
		m_entities.push_back(entity);

		return m_dense.back();
	}

	void Remove(Entity entity)
	{
		uint32_t idx = entity.Index();
		assert(m_sparse[idx] != INVALID && "Entity does not have this component");

		uint32_t denseIdx = m_sparse[idx];
		uint32_t lastDenseIdx = static_cast<uint32_t>(m_dense.size()) - 1;

		// Swap the removed element with the last - O(1) erase
		m_dense[denseIdx] = std::move(m_dense[lastDenseIdx]);
		m_entities[denseIdx] = m_entities[lastDenseIdx];

		// Fix up the sparse entry for the entity we just moved
		m_sparse[m_entities[denseIdx].Index()] = denseIdx;

		// Clear the removed entity's sparse entry
		m_sparse[idx] = INVALID;
		m_dense.pop_back();
		m_entities.pop_back();
	}

	// Called by ComponentStore when an entity is destroyed
	// safe to call even if the entity never had this component
	void RemoveIfExists(Entity entity) override
	{
		if (Has(entity)) Remove(entity);
	}

	T& Get(Entity entity)
	{
		assert(Has(entity) && "Entity does not have this component");
		return m_dense[m_sparse[entity.Index()]];
	}
	
	// Const accessor for read-only access to components
	const T& Get(Entity entity) const
	{
		assert(Has(entity) && "Entity does not have this component");
		return m_dense[m_sparse[entity.Index()]];
	}

	const std::vector<Entity>& GetEntities() const
	{
		return m_entities;
	}

	bool Has(Entity entity) const
	{
		uint32_t idx = entity.Index();
		return idx < m_sparse.size() && m_sparse[idx] != INVALID;
	}

	// Iteration - tight packed loop, no gaps
	T* Data() { return m_dense.data(); }
	uint32_t Count() const { return static_cast<uint32_t>(m_dense.size()); }

	// Range-based for support

	auto begin() { return m_dense.begin(); }
	auto end() { return m_dense.end(); }

private:
	static constexpr uint32_t INVALID = UINT32_MAX;

	std::vector<T> m_dense; // packed components
	std::vector<Entity> m_entities; // parallel entity handles
	std::vector<uint32_t> m_sparse; // entity index -> dense index
};