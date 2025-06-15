#include "engine.h"

#include <iostream>
#include <format>
#include <application/system/window_system.h>

Engine::Engine()
{
	WindowInfo winInfo = {
		.m_windowName = "Simple",
		.m_width = 1200,
		.m_height = 800
	};
	m_systemHolder.addSystem<WindowSystem>(std::move(winInfo));
}

Engine::~Engine()
{
}

void Engine::run()
{
	while (true)
	{
		for (auto& [name, system] : m_systemHolder)
		{
			system.update(0.0f);
		}
	}
}
