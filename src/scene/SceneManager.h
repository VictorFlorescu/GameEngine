#pragma once
#include "../ecs/Registry.h"
#include "../assets/AssetManager.h"
#include <memory>
#include <vector>
#include <functional>
#include <cassert>

// Scene base class - all scenes inherit from here

class Scene
{
public:
	virtual ~Scene() = default;

	virtual void OnEnter(Registry& registry, AssetManager& assets) {}
	virtual void OnExit(Registry& registry, AssetManager& assets) {}
	virtual void OnPause(Registry& registry) {}
	virtual void OnResume(Registry& registry) {}
	virtual void Update(float deltaTime, Registry& registry, AssetManager& assets) {}
};

// SceneManager

class SceneManager
{
public:
	// push a new scene onto the stack - pauses the current one
	// Usage: sceneManager.Push<MyScene>(registry, assets, ctorArgs...);

	template<typename T, typename ... Args>
	void Push(Registry& registry, AssetManager& assets, Args&&... args)
	{
		Enqueue([&, args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
			if (!m_stack.empty())
				m_stack.back()->OnPause(registry);

			auto scene = std::apply([](auto&&... a)
				{
					return std::make_unique<T>(std::forward<decltype(a)>(a)...);
				}, std::move(args));

			scene->OnEnter(registry, assets);
			m_stack.push_back(std::move(scene));
			});
	}

	// Pop the top scene and resume the one below it
	void Pop(Registry& registry, AssetManager& assets)
	{
		Enqueue([&]() {
			if (m_stack.empty()) return;

			m_stack.back()->OnExit(registry, assets);
			m_stack.pop_back();

			if (!m_stack.empty())
				m_stack.back()->OnResume(registry);
			});
	}

	// Replace the top scene with a new one (no pause/resume on stack below)

	template<typename T, typename... Args>
	void Replace(Registry& registry, AssetManager& assets, Args&&... args)
	{
		Enqueue([&, args = std::make_tuple(std::forward<Args>(args)...)]() mutable
			{
				if (!m_stack.empty())
				{
					m_stack.back()->OnExit(registry, assets);
					m_stack.pop_back();
				}

				auto scene = std::apply([](auto&&... a)
					{
						return std::make_unique<T>(std::forward<decltype(a)>(a)...);
					}, std::move(args));

				scene->OnEnter(registry, assets);
				m_stack.push_back(std::move(scene));
			});
	}

	// Clear the entire stack
	void Clear(Registry& registry, AssetManager& assets)
	{
		Enqueue([&]()
			{
				while (!m_stack.empty())
				{
					m_stack.back()->OnExit(registry, assets);
					m_stack.pop_back();
				}
			});
	}

	// Called every frame by Application::Tick
	void Update(float deltaTime, Registry& registry, AssetManager& assets)
	{
		// flush pending transitions first so they take effect before this frame's update
		FlushPending(registry, assets);

		if (!m_stack.empty())
			m_stack.back()->Update(deltaTime, registry, assets);
	}
	
	bool IsEmpty() const { return m_stack.empty(); }
	size_t Size() const { return m_stack.size(); }

	// Peek at the top scene without taking ownership
	Scene* Top() const
	{
		return m_stack.empty() ? nullptr : m_stack.back().get();
	}

private:
	std::vector<std::unique_ptr<Scene>> m_stack;
	std::vector<std::function<void()>> m_pending; // deferred transitions

	// transitions are deferred to avoid modifying the stack mid-update
	// this prevents use-after-free if a scene pushes/pops during its own update
	void Enqueue(std::function<void()> fn)
	{
		m_pending.push_back(std::move(fn));
	}

	void FlushPending(Registry& registry, AssetManager& assets)
	{
		// swap out the list before flushing - a transition can itself
		// enqueue more transitions, which run next frame
		auto toRun = std::move(m_pending);
		m_pending.clear();
		for (auto& fn : toRun) fn();
	}
};