#pragma once

#include <application/core/system_interface.h>
#include <application/core/manager_interface.h>

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
	
	bool			m_running = true;
};