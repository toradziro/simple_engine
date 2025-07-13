export module renderer_manager;

import <vector>;
import <string>;
import <array>;
import <functional>;
import <glm/glm.hpp>;

import <vulkan/vulkan.h>;
import imgui_integration;

export struct VertexData
{
	glm::vec4	m_vertex;
	glm::vec3	m_color;
	glm::vec2	m_texCoord;
};

export struct SpriteInfo
{
	glm::vec3					m_position;
	std::string					m_texturePath;
};

export struct RendererManager
{
	using ImGuiDrawCallback = std::function<void()>;

	void addSpriteToDrawList(const SpriteInfo& spriteInfo)
	{
		m_sprites.push_back(spriteInfo);
	}

	void addImGuiDrawCallback(ImGuiDrawCallback imGuiUpdateUi)
	{
		m_imGuiUpdatesUi.emplace_back(imGuiUpdateUi);
	}

	//-- User notation object
	std::vector<SpriteInfo>			m_sprites;
	std::vector<ImGuiDrawCallback>	m_imGuiUpdatesUi;
};