#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <imgui.h>
#include <format>

#include <application/managers/renderer_manager.h>

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
	EngineContext& m_engineContext;

	//-- test
	SpriteInfo m_firstSprite;
	SpriteInfo m_secondSprite;
	float      m_fps;
};
