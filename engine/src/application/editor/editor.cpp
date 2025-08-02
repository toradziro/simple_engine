#include "editor.h"

#include <application/core/event_interface.h>
#include <application/engine_context.h>
#include <application/managers/renderer_manager.h>

//-------------------------------------------------------------------------------------------------
EditorSystem::EditorSystem(std::shared_ptr<EngineContext> context) : m_engineContext(context)
{
	m_editorContext = std::make_shared<EditorContext>();
	m_editorContext->m_currentScene = std::make_unique<Scene>(m_engineContext);

	m_scenePanel = ScenePanel(m_editorContext);
	m_entityPanel = EntityPanel(m_editorContext);

	m_firstEnt = std::make_unique<Entity>(m_editorContext->m_currentScene->addEntity());
	m_secondEnt = std::make_unique<Entity>(m_editorContext->m_currentScene->addEntity());

	m_firstEnt->component<TransformComponent>().m_position = { 0.5f, 0.5f, 0.5f };
	m_secondEnt->component<TransformComponent>().m_position = { 0.0f, 0.0f, 0.0f };

	m_firstEnt->addComponent<SpriteComponent>("images/nyan_cat.png");
	m_secondEnt->addComponent<SpriteComponent>("images/gg2.png");
}

//-------------------------------------------------------------------------------------------------
void EditorSystem::update(float dt)
{
	m_fps = 1.0f / dt;

	m_editorContext->m_currentScene->update(dt);

	auto& rendererManager = m_engineContext->m_managerHolder.getManager<RendererManager>();
	rendererManager.addImGuiDrawCallback([this]()
		{
			m_scenePanel.update();
			m_entityPanel.update();
			updateUI();
		});
}

//-------------------------------------------------------------------------------------------------
void EditorSystem::updateUI()
{
	//-- Test integration
	if (ImGui::Begin("Test Window"))
	{
		if (ImGui::Button("Switch Textures"))
		{
			std::swap(m_firstEnt->component<SpriteComponent>().m_texturePath
				, m_secondEnt->component<SpriteComponent>().m_texturePath);
		}

		ImGui::End();
	}

	if (ImGui::Begin("Statistics info"))
	{
		ImGui::Text("FPS: %d", static_cast<int>(m_fps));

		//ImGui::Text("Current Scene: %s", m_context->m_currentScene->name().c_str());
		if (m_editorContext->m_selectedEntity)
		{
			std::string name = m_editorContext->m_selectedEntity->component<EntityName>().m_name;
			ImGui::Text("Selected entity: %s", name.c_str());
		}
		else
		{
			ImGui::Text("Selected entity: <none>");
		}
		ImGui::End();
	}
}
