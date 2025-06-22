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
	while (true)
	{
		for (auto& [name, system] : m_systemHolder)
		{
			system.update(0.0f);

			//-- TEST, INCAPSULATE LATER!!!
			auto& rendererSystem = m_systemHolder.getSystem<Renderer>();
			rendererSystem.beginFrame(0.0f);
			rendererSystem.drawSprite(m_firstSprite);
			rendererSystem.drawSprite(m_secondSprite);
			//-- drawSprite will got here
			rendererSystem.endFrame();
		}
	}
}
