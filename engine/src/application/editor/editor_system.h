#pragma once

#include <application/managers/events/event_interface.h>
//-- TODO: Move to cpp later when no need in test sprites will be
#include <application/managers/renderer_manager.h>

struct EngineContext;

class EditorSystem
{
public:
	EditorSystem(EngineContext& context);

	void	update(float dt);
	void	onEvent(Event& event) const;

private:
	void	updateUI();

	EngineContext&	m_engineContext;

	//-- test
	SpriteInfo		m_firstSprite;
	SpriteInfo		m_secondSprite;
};