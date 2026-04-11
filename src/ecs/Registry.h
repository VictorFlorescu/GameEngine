#pragma once
#include "ComponentStore.h"
#include <functional>
#include <algorithm>

class Registry
{
public:
	explicit Registry(uint32_t maxEntities = Entity::MAX_ENTITIES) : m_entities(maxEntities), m_store(maxEntities) {}

	// Entity lifetime

	Entity Create()
	{
		return m_entities.Create();
	}

	void Destroy(Entity entity)
	{
		assert(IsAlive(entity) && "Destroying a dead entity");
		m_store.OnEntityDestroyed(entity);
		m_entities.Destroy(entity);
	}

	bool IsAlive(Entity entity) const
	{
		return m_entities.IsAlive(entity);
	}

	uint32_t AliveCount() const
	{
		return m_entities.AliveCount();
	}

	// Component registration

	template<typename T>
	void RegisterComponent()
	{
		m_store.Register<T>();
	}

	// Register multiple types at once
	template<typename... Ts>
	void RegisterComponents()
	{
		(RegisterComponent<Ts>(), ...);
	}

	// Component access

	template<typename T, typename... Args>
	T& Emplace(Entity entity, Args&&... args)
	{
		assert(IsAlive(entity));
		return m_store.Emplace<T>(entity, std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	T& EmplaceOrReplace(Entity entity, Args&&... args)
	{
		assert(IsAlive(entity));
		if (m_store.Has<T>(entity))
			m_store.Remove<T>(entity);
		return m_store.Emplace<T>(entity, std::forward<Args>(args)...);
	}

	template<typename T>
	void Remove(Entity entity)
	{
		assert(IsAlive(entity));
		m_store.Remove<T>(entity);
	}

	template<typename T>
	T& Get(Entity entity)
	{
		assert(IsAlive(entity));
		return m_store.Get<T>(entity);
	}

	template<typename T>
	const T& Get(Entity entity) const
	{
		assert(IsAlive(entity));
		return m_store.Get<T>(entity);
	}

	// Returns nullptr if the entity doesn't have the component

	template<typename T>
	T* TryGet(Entity entity)
	{
		if (!IsAlive(entity) || !m_store.Has<T>(entity)) return nullptr;
		return &m_store.Get<T>(entity);
	}

	template<typename T>
	bool Has(Entity entity) const
	{
		return IsAlive(entity) && m_store.Has<T>(entity);
	}

	// View - iterate entities that have ALL of Ts...

	// Callback form: registry.View<Transform, Sprite>([](Entity e, Transform& t, Sprite& s){ ... })

	template<typename... Ts, typename Func>
	void View(Func&& func)
	{
		static_assert(sizeof...(Ts) > 0, "View requires at least one component type");

		// Pick the smallest array to iterate - avoids looping over large arrays
		// when a rare component narrows the candidate set drastically
		
		ComponentArray<uint8_t>* smallest = FindSmallest<Ts...>();
		(void)smallest; // used only for size compare; iteration is below

		// Iterate the smallest array by dispatching to the right typed overload
		IterateSmallest<Func, Ts...>(std::forward<Func>(func));
	}

	// Struct-based view for range-for loops (single component only)
	// For multi-component iteration use callback form above
	template<typename T>
	ComponentArray<T>& Each()
	{
		return m_store.GetArray<T>();
	}

	// Direct store access (for systems that need raw arrays)

	ComponentStore& GetStore() { return m_store; }

private:
	EntityManager m_entities;
	ComponentStore m_store;

	// View internals

	// Returns the size of the smallest component array among Ts...
	// Used to pick which array to iterate.

	template<typename... Ts>
	uint32_t SmallestSize() const
	{
		uint32_t sizes[] = { m_store.GetArray<Ts>().Count()... };
		return *std::min_element(std::begin(sizes), std::end(sizes));
	}

	template<typename Func, typename First, typename... Rest>
	void IterateSmallest(Func&& func)
	{
		uint32_t firstSize = m_store.GetArray<First>().Count();

		// If another type's array is smaller, rotate and retry
		// This expands to a compile-time chain of size comparisons

		if constexpr (sizeof...(Rest) > 0)
		{
			if (ShouldRotate<First, Rest...>())
			{
				IterateSmallest<Func, Rest..., First>(std::forward<Func>(func));
				return;
			}
		}

		// First is the smallest; iterate it and filter by Rest...
		auto& firstArray = m_store.GetArray<First>();
		auto& entities = firstArray.GetEntities(); // parallel entity array

		for (uint32_t i = 0; i < firstArray.Count(); ++i)
		{
			Entity e = entities[i];
			if ((m_store.Has<Rest>(e) && ...))
			{
				func(e, firstArray.Data()[i], m_store.Get<Rest>(e)...);
			}	
		}
	}

	// Returns true if any type in Rest... has a smaller array than First

	template<typename First, typename... Rest>
	bool ShouldRotate() const
	{
		uint32_t firstSize = m_store.GetArray<First>().Count();
		uint32_t restSizes[] = { m_store.GetArray<Rest>().Count()... };
		for (uint32_t s : restSizes)
			if (s < firstSize) return true;
		return false;
	}
	

};