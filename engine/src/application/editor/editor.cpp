#include "editor.h"

#include <application/core/event_interface.h>
#include <application/engine_context.h>

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
EditorSystem::EditorSystem(EngineContext& context) : m_engineContext(context)
{
	m_firstSprite = { { 0.5f, 0.5f, 0.5f }, "images/nyan_cat.png" };
	m_secondSprite = { { 0.0f, 0.0f, 0.0f }, "images/gg2.png" };
}

//-------------------------------------------------------------------------------------------------
void EditorSystem::update(float dt)
{
	m_fps = 1.0f / dt;

	auto& rendererManager = m_engineContext.m_managerHolder.getManager<RendererManager>();
	rendererManager.addImGuiDrawCallback([this]()
		{
			updateUI();
		});

	//-- Tst drawing here
	rendererManager.addSpriteToDrawList(m_firstSprite);
	rendererManager.addSpriteToDrawList(m_secondSprite);
}

//-------------------------------------------------------------------------------------------------
void EditorSystem::updateUI()
{
	//-- Test integration
	if (ImGui::Begin("Test Window"))
	{
		if (ImGui::Button("Switch Textures"))
		{
			std::string tmp = m_firstSprite.m_texturePath;
			m_firstSprite.m_texturePath = m_secondSprite.m_texturePath;
			m_secondSprite.m_texturePath = tmp;
		}

		drawVec3Prop(m_firstSprite.m_position, "f_sprite");
		drawVec3Prop(m_secondSprite.m_position, "s_sprite");
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
