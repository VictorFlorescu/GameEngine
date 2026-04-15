#pragma once
#include "IComponentArray.h"
#include "ComponentArray.h"
#include "Entity.h"
#include "EntityManager.h"
#include <unordered_map>
#include <memory>
#include <typeindex>

class ComponentStore
{
public:
	explicit ComponentStore(uint32_t maxEntities) : m_maxEntities(maxEntities) {}

	// Register a component type before use (call once at startup)
	template<typename T>
	void Register()
	{
		auto key = std::type_index(typeid(T));
		assert(m_arrays.find(key) == m_arrays.end() && "Component already registered");
		m_arrays[key] = std::make_unique<ComponentArray<T>>(m_maxEntities);
	}

	template<typename T, typename... Args>
	T& Emplace(Entity entity, Args&&... args)
	{
		return GetArray<T>().Emplace(entity, std::forward<Args>(args)...);
	}

	template<typename T>
	void Remove(Entity entity)
	{
		GetArray<T>().Remove(entity);
	}

	template<typename T>
	T& Get(Entity entity)
	{
		return GetArray<T>().Get(entity);
	}

	template<typename T>
	bool Has(Entity entity) const
	{
		return GetArray<T>().Has(entity);
	}

	// Called by Registry::Destroy - cleans up all components for the entity
	void OnEntityDestroyed(Entity entity)
	{
		for (auto& [key, array] : m_arrays)
			array->RemoveIfExists(entity);
	}

	template<typename T>
	ComponentArray<T>& GetArray()
	{
		auto key = std::type_index(typeid(T));
		auto it = m_arrays.find(key);
		assert(it != m_arrays.end() && "Component type not registered");
		return static_cast<ComponentArray<T>&>(*it->second);
	}
	template<typename T>
	const ComponentArray<T>& GetArray() const
	{
		auto key = std::type_index(typeid(T));
		auto it = m_arrays.find(key);
		assert(it != m_arrays.end() && "Component type not registered");
		return static_cast<const ComponentArray<T>&>(*it->second);
	}

private:
	uint32_t m_maxEntities;
	std::unordered_map < std::type_index, std::unique_ptr<IComponentArray>> m_arrays;
};