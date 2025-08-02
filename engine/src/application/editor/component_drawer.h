#pragma once

#include <application/core/scene/component.h>
#include <application/core/scene/scene.h>
#include <application/core/utils/engine_assert.h>

#include "imgui_helpers.h"

struct ComponentDrawer
{

	template<typename Component>
	void draw(Entity& innerEntity, Scene& m_scene)
	{
		engineAssert(false, "Provide specialization for the component");
	}

};

template<>
void ComponentDrawer::draw<EntityName>(Entity& innerEntity, Scene& m_scene)
{
	if (!innerEntity.hasComponent<EntityName>())
	{
		return;
	}
	auto& comp = innerEntity.component<EntityName>();
	const std::string imGuiId = std::format("##{}{}str", "EntityName", static_cast<uint32_t>(innerEntity.entityId()));

	const int32_t C_BUF_LENGTH = 1024;

	char buff[C_BUF_LENGTH];
	memset(buff, 0, C_BUF_LENGTH);
	std::strcpy(buff, comp.m_name.c_str());

	ImGui::PushItemWidth(-FLT_MIN);
	if (ImGui::InputText(imGuiId.c_str(), buff, C_BUF_LENGTH))
	{
		comp.m_name = std::string(buff);
	}
	ImGui::PopItemWidth();
}

template<>
void ComponentDrawer::draw<TransformComponent>(Entity& innerEntity, Scene& m_scene)
{
	if (!innerEntity.hasComponent<TransformComponent>())
	{
		return;
	}
	auto& comp = innerEntity.component<TransformComponent>();

	const std::string imGuiId = std::format("##{}{}str", "TransformComponent", static_cast<uint32_t>(innerEntity.entityId()));
	drawVec3Prop(comp.m_position, imGuiId.c_str());
}

template<>
void ComponentDrawer::draw<SpriteComponent>(Entity& innerEntity, Scene& m_scene)
{
	if (!innerEntity.hasComponent<SpriteComponent>())
	{
		return;
	}
	innerEntity.component<SpriteComponent>();
}

template<>
void ComponentDrawer::draw<CameraComponent>(Entity& innerEntity, Scene& m_scene)
{
	if (!innerEntity.hasComponent<CameraComponent>())
	{
		return;
	}
	innerEntity.component<CameraComponent>();
}
