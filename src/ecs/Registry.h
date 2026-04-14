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

		// Build a tuple of pointers to each ComponentArray 
		// resolved once here, not on every iteration
		auto arrays = std::make_tuple(&m_store.GetArray<Ts>()...);

		// Find the index of the smallest array among Ts... at compile time using a fold over an index sequence
		constexpr size_t N = sizeof...(Ts);
		size_t sizes[N] = { m_store.GetArray<Ts>().Count()... };

		size_t smallestIdx = 0;
		for (size_t i = 1; i < N; ++i)
			if (sizes[i] < sizes[smallestIdx])
				smallestIdx = i; 

		// Dispatch to the typed iterator for whichever index is smallest
		IterateAt<Func, Ts...>(
			smallestIdx,
			arrays,
			std::forward<Func>(func),
			std::index_sequence_for<Ts...>{}
		);
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

	// Entry point: given a runtime smallestIdx, walk the index sequence 
	// and call the right typed overload via if constexpr
	template<typename Func, typename... Ts, size_t... Is>
	void IterateAt(
		size_t smallestIdx,
		std::tuple<ComponentArray<Ts>*...>& arrays,
		Func&& func,
		std::index_sequence<Is...>
	)
	{
		// Expands to: if (smallestIdx == 0) IterateOver<0, Ts...>(...);
		//             if (smallestIdx == 1) IterateOver<1, Ts...>(...);
		//             ...
		// Only one branch executes; the rest are compiled away.
		(void)std::initializer_list<int>
		{
			(smallestIdx == Is
				? (IterateOver<Is, Func, Ts...>(arrays, std::forward<Func>(func), std::index_sequence_for<Ts...>{}), 0)
				: 0)...
		};
	}

	// Core loop: iterate array[PivotIdx], check all others via Has<>
	template<size_t PivotIdx, typename Func, typename... Ts, size_t... Is>
	void IterateOver(
		std::tuple<ComponentArray<Ts>*...>& arrays,
		Func&& func,
		std::index_sequence<Is...>
	)
	{
		using PivotType = std::tuple_element_t<PivotIdx, std::tuple<Ts...>>;

		auto* pivotArray = std::get<PivotIdx>(arrays);
		const auto& entities = pivotArray->GetEntities();

		for (uint32_t i = 0; i < pivotArray->Count(); ++i)
		{
			Entity entity = entities[i];

			// Check every array EXCEPT the pivot - it already
			// has the entity by definition
			bool allMatch = ((Is == PivotIdx || std::get<Is>(arrays)->Has(entity)) && ...);
			if (!allMatch) continue;

			// Call func(entity, component0&, component1&...)
			// Each component is fetched from its own array - no branch on pivot
			func(entity, std::get<Is>(arrays)->Get(entity)...);
		}
	}
};