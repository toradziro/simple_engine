#pragma once

#include <application/core/system_interface.h>
#include <application/core/manager_interface.h>
//-- TODO: Move to cpp later when no need in test sprites will be
#include <application/managers/renderer_manager.h>

struct EngineContext
{
	ManagerHolder	m_managerHolder;
};

class Engine
{
public:
	Engine();
	~Engine();

	void	run();

private:
	EngineContext	m_context;
	SystemHolder	m_systemHolder;

	//-- test
	SpriteInfo		m_firstSprite;
	SpriteInfo		m_secondSprite;
	
	bool			m_running = true;
};