#include "editor_system.h"
#include <application/engine.h>
#include <imgui.h>

EditorSystem::EditorSystem(EngineContext& context) : m_engineContext(context)
{
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

void EditorSystem::update(float dt)
{
	auto& rendererManager = m_engineContext.m_managerHolder.getManager<RendererManager>();
	rendererManager.addImGuiDrawCallback([this]()
		{
			updateUI();
		});

	//-- Tst drawing here
	rendererManager.addSpriteToDrawList(m_firstSprite);
	rendererManager.addSpriteToDrawList(m_secondSprite);
}

void EditorSystem::onEvent(Event& event) const
{
}

void EditorSystem::updateUI()
{
	//-- Test integration
	ImGui::Begin("Test Window");
	if (ImGui::Button("Switch Textures"))
	{
		std::string tmp = m_firstSprite.m_texturePath;
		m_firstSprite.m_texturePath = m_secondSprite.m_texturePath;
		m_secondSprite.m_texturePath = tmp;
	}
	ImGui::End();
}
