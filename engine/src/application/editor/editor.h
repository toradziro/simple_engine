#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <imgui.h>
#include <format>

#include <application/core/scene/scene.h>

class Event;
struct EngineContext;

class EditorSystem
{
public:
	EditorSystem(EngineContext& context);

	void update(float dt);
	void onEvent(Event& event) const {}

private:
	void updateUI();

private:
	EngineContext&	m_engineContext;
	Scene			m_scene;

	//-- Test data, remove later
	std::unique_ptr<Entity>	m_firstEnt;
	std::unique_ptr<Entity>	m_secondEnt;

	float			m_fps = 0.0f;
};
