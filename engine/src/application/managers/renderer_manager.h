#pragma once

#include <vector>
#include <string>
#include <array>
#include <functional>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

//-------------------------------------------------------------------------------------------------
struct VertexData
{
	glm::vec4 m_vertex;
	glm::vec3 m_color;
	glm::vec2 m_texCoord;
};

//-------------------------------------------------------------------------------------------------
struct SpriteInfo
{
	glm::vec3   m_position;
	std::string m_texturePath;
};

//-------------------------------------------------------------------------------------------------
struct RendererManager
{
	using ImGuiDrawCallback = std::function<void()>;

	//-------------------------------------------------------------------------------------------------
	void addSpriteToDrawList(const SpriteInfo& spriteInfo);

	//-------------------------------------------------------------------------------------------------
	void addImGuiDrawCallback(ImGuiDrawCallback imGuiUpdateUi);

	//-- User notation object
	std::vector<SpriteInfo>        m_sprites;
	std::vector<ImGuiDrawCallback> m_imGuiUpdatesUi;
};
