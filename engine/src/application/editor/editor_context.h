#pragma once

#include <application/core/scene/scene.h>

#include <memory>

struct EditorContext
{
	std::unique_ptr<Scene>	m_currentScene;
	std::unique_ptr<Entity>	m_selectedEntity;
};