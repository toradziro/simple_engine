#pragma once

#include <glm/glm.hpp>
#include <string>

struct EntityName
{
	std::string m_name;
};

struct TransformComponent
{
	glm::vec3 m_position;
};

struct SpriteComponent
{
	std::string m_texturePath;
};

struct CameraComponent
{
	glm::mat4 m_cameraMat = { 1.0f };
};
