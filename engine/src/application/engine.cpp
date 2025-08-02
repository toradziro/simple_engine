#include "engine.h"

#include <application/managers/window_manager.h>
#include <application/system/window_system.h>
#include <application/core/event_interface.h>
#include <application/renderer/renderer.h>
#include <application/managers/renderer_manager.h>
#include <application/editor/editor.h>
#include <application/core/manager_interface.h>
#include <application/managers/virtual_fs.h>

//-------------------------------------------------------------------------------------------------
Engine::Engine(const Config& config)
{
	WindowInfo winInfo = {
		.m_windowName = "Simple"
		, .m_width = 1200
		, .m_height = 800
		, .m_eventCallback = [&](Event& event)
		{
			eventCallback(event);
		}
	};

	m_context = std::make_shared<EngineContext>();

	//-- Create managers, be aware that managers may be initialized inside corresponding systems
	m_context->m_managerHolder.addManager<WindowManager>();
	m_context->m_managerHolder.addManager<RendererManager>();
	m_context->m_managerHolder.addManager<VirtualFS>(config.m_projectPath);

	//-- Create systems
	m_systemHolder.addSystem<WindowSystem>(m_context, std::move(winInfo));
	m_systemHolder.addSystem<RendererSystem>(m_context);
	m_systemHolder.addSystem<EditorSystem>(m_context);
}

//-------------------------------------------------------------------------------------------------
void Engine::run()
{
	float lastFrameDt = 0.0f;

	while (m_running)
	{
		auto  timeStart = absl::Now();
		auto& rendererManager = m_context->m_managerHolder.getManager<RendererManager>();

		for (auto& [name, system] : m_systemHolder)
		{
			system.update(lastFrameDt);
		}

		auto timeEnd = absl::Now();
		lastFrameDt = absl::ToDoubleSeconds(timeEnd - timeStart);
	}
}

void Engine::eventCallback(Event& event)
{
	if (eventTypeCheck<WindowCloseEvent>(event))
	{
		m_running = false;
		event.setHandeled();
		std::println("WindowCloseEvent");
	}

	//-- Send events to systems
	for (auto& [_, system] : m_systemHolder)
	{
		system.onEvent(event);
	}
}
