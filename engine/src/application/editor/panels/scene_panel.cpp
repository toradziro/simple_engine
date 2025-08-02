#include "scene_panel.h"
#include <imgui.h>

void ScenePanel::update()
{
	if (ImGui::Begin("Scene Hierarchy"))
	{
		drawContextMenu();

		entt::registry& registry = m_editorContext->m_currentScene->registry();
		registry.view<entt::entity>().each([&](entt::entity entity)
			{
				Entity innerEntity(entity, registry);

				const std::string_view entityName = innerEntity.component<EntityName>().m_name;
				int flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth;
				if (m_editorContext->m_selectedEntity && m_editorContext->m_selectedEntity->entityId() == entity)
				{
					flags |= ImGuiTreeNodeFlags_Selected;
				}
				auto modifiedName = std::format("{}##{}", entityName.empty() ? "_" : entityName.data(), static_cast<uint64_t>(entity));
				bool nodeExpanded = ImGui::TreeNodeEx(modifiedName.c_str(), flags);
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right))
				{
					selectEntity(innerEntity);
				}
				if (ImGui::IsKeyPressed(ImGuiKey_Escape))
				{
					resetSelection();
				}
				if (nodeExpanded)
				{
					ImGui::TreePop();
				}
			});
	}
	ImGui::End();
}

void ScenePanel::drawContextMenu()
{
	const ImVec2 regionMin = ImGui::GetWindowContentRegionMin();
	const ImVec2 regionMax = ImGui::GetWindowContentRegionMax();
	const ImVec2 mousePos = ImGui::GetMousePos();

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
	{
		resetSelection();
		ImGui::OpenPopup("##contextMenu");
	}
	if (ImGui::BeginPopupContextItem("##contextMenu"))
	{
		const bool anyEntitySelected = (m_editorContext->m_selectedEntity) && (m_editorContext->m_selectedEntity->entityId() != entt::null);
		if (anyEntitySelected)
		{
			if (ImGui::MenuItem("Remove Entity"))
			{
				m_editorContext->m_currentScene->removeEntity(m_editorContext->m_selectedEntity->entityId());
				m_editorContext->m_selectedEntity.reset();
			}
		}
		else
		{
			if (ImGui::MenuItem("New Entity"))
			{
				auto& scene = m_editorContext->m_currentScene;
				auto newEntity = scene->addEntity();
				newEntity.component<EntityName>().m_name = "New entity";
				selectEntity(newEntity);
			}
		}
		ImGui::EndPopup();
	}
}

void ScenePanel::resetSelection()
{
	m_editorContext->m_selectedEntity.reset();
}

void ScenePanel::selectEntity(Entity e)
{
	m_editorContext->m_selectedEntity = std::make_unique<Entity>(e);
}