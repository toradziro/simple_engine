#pragma once

#include <vector>
#include <string>
#include <array>
#include <functional>
#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

class RendererSystem;
class ImGuiIntegration;

struct VertexData
{
	glm::vec4	m_vertex;
	glm::vec3	m_color;
	glm::vec2	m_texCoord;
};

struct SpriteInfo
{
	glm::vec3					m_position;
	std::string					m_texturePath;
	//std::array<VertexData, 4>	m_verticies;
};

class RendererManager
{
public:
	using ImGuiDrawCallback = std::function<void()>;

	void addSpriteToDrawList(const SpriteInfo& spriteInfo)
	{
		m_sprites.push_back(spriteInfo);
	}

	void addImGuiDrawCallback(ImGuiDrawCallback imGuiUpdateUi)
	{
		m_imGuiUpdatesUi.emplace_back(imGuiUpdateUi);
	}

private:
	friend class RendererSystem;
	friend class ImGuiIntegration;

	//-- User notation object
	std::vector<SpriteInfo>			m_sprites;
	std::vector<ImGuiDrawCallback>	m_imGuiUpdatesUi;
};