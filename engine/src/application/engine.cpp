#include "engine.h"

#include <iostream>
#include <format>
#include <chrono>
#include <ctime>
#include <application/managers/window_manager.h>
#include <application/system/window_system.h>
#include <renderer/renderer.h>

Engine::Engine()
{
	WindowInfo winInfo = {
		.m_windowName = "Simple",
		.m_width = 1200,
		.m_height = 800
	};

	//-- Create managers, be aware that managers may be initialized inside corresponding systems
	m_context.m_managerHolder.addManager<WindowManager>();
	m_context.m_managerHolder.addManager<RendererManager>();

	//-- Create systems
	m_systemHolder.addSystem<WindowSystem>(m_context, std::move(winInfo));
	m_systemHolder.addSystem<RendererSystem>(m_context);

	//-- Test sprites
	const std::array<VertexData, 4> firstQuad = {
		VertexData{{-0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }},
		VertexData{{0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, { 1.0f, 0.0f }},
		VertexData{{0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, { 1.0f, 1.0f }},
		VertexData{{-0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, { 0.0f, 1.0f }}
	};
	const std::array<VertexData, 4> secondQuad = {
		VertexData{{0.25f, -0.5f}, {0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }},
		VertexData{{1.25f, -0.5f}, {0.0f, 0.0f, 0.0f}, { 1.0f, 0.0f }},
		VertexData{{1.25f, 0.5f}, {0.0f, 0.0f, 0.0f}, { 1.0f, 1.0f }},
		VertexData{{0.25f, 0.5f}, {0.0f, 0.0f, 0.0f}, { 0.0f, 1.0f }}
	};

	m_firstSprite = { firstQuad, "images/nyan_cat.png" };
	m_secondSprite = { secondQuad, "images/gg2.png" };
}

Engine::~Engine()
{
}

void Engine::run()
{
	float lastFrameDt = 0.0f;
	while (true)
	{
		auto t_start = std::chrono::high_resolution_clock::now();
		auto& rendererManager = m_context.m_managerHolder.getManager<RendererManager>();
		
		//-- Tst drawind here
		rendererManager.addSpriteToDrawList(m_firstSprite);
		rendererManager.addSpriteToDrawList(m_secondSprite);
		
		for (auto& [name, system] : m_systemHolder)
		{
			system.update(lastFrameDt);
		}

		const auto t_end = std::chrono::high_resolution_clock::now();
		lastFrameDt = std::chrono::duration<float>(t_end - t_start).count();
	}
}
