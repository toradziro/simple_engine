#include "engine.h"

#include <iostream>
#include <format>
#include <chrono>
#include <ctime>
#include <print>

#include <application/managers/window_manager.h>
#include <application/system/window_system.h>
#include <application/managers/events/events_types.h>
#include <renderer/renderer.h>
#include <application/managers/renderer_manager.h>
#include <application/editor/editor_system.h>

Engine::Engine()
{
	WindowInfo winInfo = {
		.m_windowName = "Simple",
		.m_width = 1200,
		.m_height = 800,
		.m_eventCallback = [&](Event& event)
			{
				if (eventTypeCheck<WindowCloseEvent>(event))
				{
					m_running = false;
					event.setHandeled();
					std::println("WindowCloseEvent");
				}
				for (auto& [_, system] : m_systemHolder)
				{
					system.onEvent(event);
				}
			}
	};

	//-- Create managers, be aware that managers may be initialized inside corresponding systems
	m_context.m_managerHolder.addManager<WindowManager>();
	m_context.m_managerHolder.addManager<RendererManager>();

	//-- Create systems
	m_systemHolder.addSystem<WindowSystem>(m_context, std::move(winInfo));
	m_systemHolder.addSystem<RendererSystem>(m_context);
	m_systemHolder.addSystem<EditorSystem>(m_context);
}

Engine::~Engine()
{}

void Engine::run()
{
	float lastFrameDt = 0.0f;

	while (m_running)
	{
		auto t_start = std::chrono::high_resolution_clock::now();
		auto& rendererManager = m_context.m_managerHolder.getManager<RendererManager>();
		
		for (auto& [name, system] : m_systemHolder)
		{
			system.update(lastFrameDt);
		}

		const auto t_end = std::chrono::high_resolution_clock::now();
		lastFrameDt = std::chrono::duration<float>(t_end - t_start).count();
	}
}
