#include "engine.h"

#include <iostream>
#include <format>
#include <chrono>
#include <ctime>
#include <application/system/window_system.h>

Engine::Engine()
{
	WindowInfo winInfo = {
		.m_windowName = "Simple",
		.m_width = 1200,
		.m_height = 800
	};
	m_systemHolder.addSystem<WindowSystem>(std::move(winInfo));
	m_systemHolder.addSystem<Renderer>(m_systemHolder.getSystem<WindowSystem>().getWindow());

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
		auto& rendererSystem = m_systemHolder.getSystem<Renderer>();
		
		//-- Tst drawind here
		rendererSystem.drawSprite(m_firstSprite);
		rendererSystem.drawSprite(m_secondSprite);
		
		for (auto& [name, system] : m_systemHolder)
		{
			system.update(lastFrameDt);
		}
		const auto t_end = std::chrono::high_resolution_clock::now();
		lastFrameDt = std::chrono::duration<float>(t_end - t_start).count();
		std::cout << lastFrameDt << std::endl;
	}
}
