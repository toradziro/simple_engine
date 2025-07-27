#include "editor.h"

#include <application/core/event_interface.h>
#include <application/engine_context.h>
#include <application/managers/renderer_manager.h>

//-------------------------------------------------------------------------------------------------
void drawVec3Prop(glm::vec3& val, const std::string& propName)
{
	const std::string imGuiIdX = std::format("##{}float3x", propName);
	const std::string imGuiIdY = std::format("##{}float3y", propName);
	const std::string imGuiIdZ = std::format("##{}float3z", propName);

	const float lettersSize = ImGui::CalcTextSize("XYZ").x;
	const float itemWidth = (ImGui::GetColumnWidth() / 3.0f) - lettersSize;

	bool valueChanged = false;
	ImGui::AlignTextToFramePadding();
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.7f, 0.0f, 0.0f, 0.9f });
	ImGui::TextUnformatted("X");
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::PushItemWidth(itemWidth);
	valueChanged |= ImGui::DragFloat(imGuiIdX.c_str(), &val.x, 0.1f);
	ImGui::PopItemWidth();

	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.7f, 0.0f, 0.9f });
	ImGui::TextUnformatted("Y");
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::PushItemWidth(itemWidth);
	valueChanged |= ImGui::DragFloat(imGuiIdY.c_str(), &val.y, 0.1f);
	ImGui::PopItemWidth();

	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f, 0.7f, 0.9f });
	ImGui::TextUnformatted("Z");
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::PushItemWidth(itemWidth);
	valueChanged |= ImGui::DragFloat(imGuiIdZ.c_str(), &val.z, 0.1f);
	ImGui::PopItemWidth();
}

//-------------------------------------------------------------------------------------------------
EditorSystem::EditorSystem(EngineContext& context) : m_engineContext(context), m_scene(context)
{
	m_firstEnt = std::make_unique<Entity>(m_scene.addEntity());
	m_secondEnt = std::make_unique<Entity>(m_scene.addEntity());

	m_firstEnt->component<TransformComponent>().m_position = { 0.5f, 0.5f, 0.5f };
	m_secondEnt->component<TransformComponent>().m_position = { 0.0f, 0.0f, 0.0f };

	m_firstEnt->addComponent<SpriteComponent>("images/nyan_cat.png");
	m_secondEnt->addComponent<SpriteComponent>("images/gg2.png");
}

//-------------------------------------------------------------------------------------------------
void EditorSystem::update(float dt)
{
	m_fps = 1.0f / dt;

	m_scene.update(dt);

	auto& rendererManager = m_engineContext.m_managerHolder.getManager<RendererManager>();
	rendererManager.addImGuiDrawCallback([this]()
		{
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

		drawVec3Prop(m_firstEnt->component<TransformComponent>().m_position, "f_sprite");
		drawVec3Prop(m_secondEnt->component<TransformComponent>().m_position, "s_sprite");
		ImGui::End();
	}

	if (ImGui::Begin("Statistics info"))
	{
		ImGui::Text("FPS: %d", static_cast<int>(m_fps));
		ImGui::End();

		//ImGui::Text("Current Scene: %s", m_context->m_currentScene->name().c_str());
		//if (m_context->m_selectedEntity)
		//{
		//std::string name = m_context->m_selectedEntity.component<EntityNameComponent>().m_name;
		//ImGui::Text("Selected entity: %s", name.c_str());
		//}
		//else
		//{
		//ImGui::Text("Selected entity: <none>");
		//}
		//ImGui::Text("Time spent on a call: %.1f ms", m_dt * 1000.0f);
		//const auto& stat = Application::subsystems().st<Renderer2D>().stats();
		//ImGui::Text("Draw calls: %d", stat.m_drawCalls);
		//ImGui::Text("Quads count: %d", stat.m_quadCount);
	}
}
