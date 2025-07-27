#pragma once

#include <numeric>
#include <entt/entt.hpp>
#include <application/engine_context.h>
#include "component.h"

class Entity
{
public:
	explicit Entity(entt::entity entity, entt::registry& registry) : m_entity(entity), m_registry(registry) {}

	template<typename Component, typename... Args>
	void addComponent(Args&&... args)
	{
		m_registry.emplace<Component>(m_entity, std::forward<Args>(args)...);
	}

	template<typename Component>
	bool hasComponent()
	{
		return m_registry.all_of<Component>(m_entity);
	}

	template<typename Component>
	Component& component()
	{
		return m_registry.get<Component>(m_entity);
	}

private:
	entt::entity	m_entity;
	entt::registry&	m_registry;
};

class Scene
{
public:
	enum class State : uint8_t
	{
		Idle,
		Simulating
	};

	Scene(EngineContext& context) : m_engineContext(context) {}

	void update(float dt);
	void startSimulation();
	void stopSimulation();
	Entity addEntity();

private:
	EngineContext&	m_engineContext;
	//-- All entities holder
	entt::registry	m_registry;
	State			m_state = State::Idle;
};