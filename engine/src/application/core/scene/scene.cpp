#include "scene.h"
#include <application/managers/renderer_manager.h>

void Scene::update(float dt)
{
	//-- Here will go check of current state
	//-- Here we collect all entities to draw
	auto spriteView = m_registry.view<SpriteComponent>();
	sendToDraw(spriteView);
}

void Scene::startSimulation()
{
	m_state = State::Simulating;
}

void Scene::stopSimulation()
{
	m_state = State::Idle;
}

Entity Scene::addEntity()
{
	entt::entity newEntity = m_registry.create();

	Entity entity(newEntity, m_registry);
	entity.addComponent<TransformComponent>(glm::vec3(1.0f, 1.0f, 0.0f));
	entity.addComponent<EntityName>("new_entity");

	return entity;
}

void Scene::sendToDraw(auto& spriteView)
{
	for (auto& entity : spriteView)
	{
		SpriteComponent&    sprite = m_registry.get<SpriteComponent>(entity);
		TransformComponent& transform = m_registry.get<TransformComponent>(entity);
		SpriteInfo          spriteInfo{
			.m_position = transform.m_position
			, .m_texturePath = sprite.m_texturePath
		};
		m_engineContext->m_managerHolder.getManager<RendererManager>().addSpriteToDrawList(std::move(spriteInfo));
	}
}
