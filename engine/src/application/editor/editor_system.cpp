#include "editor_system.h"
#include <application/engine.h>
#include <imgui.h>

EditorSystem::EditorSystem(EngineContext& context) : m_engineContext(context)
{
	m_firstSprite  = { { 0.5f, 0.5f, 0.0f }, "images/nyan_cat.png" };
	m_secondSprite = { { 0.0f, 0.0f, 0.0f }, "images/gg2.png" };
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
