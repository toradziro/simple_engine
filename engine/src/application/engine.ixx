export module simple_engine;

import <iostream>;
import <format>;
import <chrono>;
import <ctime>;
import <print>;

import window_manager;
import window_system;
import event_interface;
import renderer_system;
import renderer_manager;
import editor;
import system_interface;
import manager_interface;
import engine_context;

export class Engine
{
public:
	Engine()
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
	~Engine()
	{
	}

	void	run()
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

private:
	EngineContext	m_context;
	SystemHolder	m_systemHolder;
	
	bool			m_running = true;
};