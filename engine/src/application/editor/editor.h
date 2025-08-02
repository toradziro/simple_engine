#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "editor_context.h"

#include <imgui.h>
#include <format>

#include <application/core/scene/scene.h>
#include <application/editor/panels/scene_panel.h>

class Event;
struct EngineContext;

class EditorSystem
{
public:
	EditorSystem(std::shared_ptr<EngineContext> context);

	void update(float dt);
	void onEvent(Event& event) const {}

private:
	void updateUI();

private:
	std::shared_ptr<EngineContext>	m_engineContext;
	std::shared_ptr<EditorContext>	m_editorContext;
	ScenePanel		m_scenePanel;

	//-- Test data, remove later
	std::unique_ptr<Entity>	m_firstEnt;
	std::unique_ptr<Entity>	m_secondEnt;

	float			m_fps = 0.0f;
};
