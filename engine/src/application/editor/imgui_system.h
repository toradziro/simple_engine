#pragma once

#include <application/managers/events/event_interface.h>

struct EngineContext;

class ImGuiSystem
{
public:
	ImGuiSystem(EngineContext& context);
	~ImGuiSystem();

	void update(float dt);
	void onEvent(Event& event);

private:
	EngineContext&	m_engineContext;
};