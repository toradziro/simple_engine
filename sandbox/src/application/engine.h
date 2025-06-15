#pragma once

#include <application/core/system_interface.h>

class Engine
{
public:
	Engine();
	~Engine();

	void	run();

private:
	SystemHolder	m_systemHolder;
};