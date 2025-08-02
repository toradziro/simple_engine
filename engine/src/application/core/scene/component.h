#pragma once

#include <glm/glm.hpp>
#include <string>

struct EntityName
{
	constexpr static inline std::string_view	C_COMPONENT_NAME = "Entity Name";

	std::string m_name;
};

struct TransformComponent
{
	constexpr static inline std::string_view	C_COMPONENT_NAME = "Transform Component";

	glm::vec3 m_position;
};

struct SpriteComponent
{
	constexpr static inline std::string_view	C_COMPONENT_NAME = "Sprite Component";

	std::string m_texturePath;
};

struct CameraComponent
{
	constexpr static inline std::string_view	C_COMPONENT_NAME = "Camera Component";

	glm::mat4 m_cameraMat = { 1.0f };
};
