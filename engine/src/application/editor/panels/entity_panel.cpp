#include "entity_panel.h"
#include <imgui.h>
#include <application/core/scene/component.h>
#include <application/editor/component_drawer.h>

void drawComponents(Entity& innerEntity, Scene& scene)
{
	ComponentDrawer drawer;
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	drawer.draw<EntityName>(innerEntity, scene);
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	drawer.draw<TransformComponent>(innerEntity, scene);
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	drawer.draw<SpriteComponent>(innerEntity, scene);
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	drawer.draw<CameraComponent>(innerEntity, scene);
}

template<typename T>
void drawAddComponent(Entity& selectedE)
{
	std::string resName = std::format("{} Component", T::C_COMPONENT_NAME);
	if (!selectedE.hasComponent<T>() && ImGui::Selectable(resName.c_str()))
	{
		selectedE.addComponent<T>();
	}
}

void EntityPanel::update()
{
	if (ImGui::Begin("Entity Panel"))
	{
		if (m_editorContext->m_selectedEntity)
		{
			constexpr auto tableName = "##conponentsTable";
			constexpr auto tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBodyUntilResize;

			if (ImGui::Button("Add Component"))
			{
				ImGui::OpenPopup("##addComponentPopup");
			}
			if (ImGui::BeginPopup("##addComponentPopup"))
			{
				auto& selectedEntity = m_editorContext->m_selectedEntity;

				drawAddComponent<EntityName>(*selectedEntity);
				drawAddComponent<TransformComponent>(*selectedEntity);
				drawAddComponent<SpriteComponent>(*selectedEntity);
				drawAddComponent<CameraComponent>(*selectedEntity);

				ImGui::EndPopup();
			}

			if (ImGui::BeginTable(tableName, 2, tableFlags, ImGui::GetContentRegionAvail()))
			{
				drawComponents(*m_editorContext->m_selectedEntity, *m_editorContext->m_currentScene);
				ImGui::EndTable();
			}
		}
		else
		{
			constexpr const char* C_NOTHING_TO_SHOW = "No entities selected";
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 textSize = ImGui::CalcTextSize(C_NOTHING_TO_SHOW);

			ImVec2 textPosition;
			textPosition.x = (windowSize.x - textSize.x) / 2.0f;
			textPosition.y = (windowSize.y - textSize.y) / 2.0f;

			ImGui::SetCursorPos(textPosition);

			ImGui::Text("%s", C_NOTHING_TO_SHOW);
		}
		ImGui::End();
	}
}
