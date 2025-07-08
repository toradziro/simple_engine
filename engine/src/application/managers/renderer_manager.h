#pragma once

#include <vector>
#include <string>
#include <array>
#include <glm/glm.hpp>

class RendererSystem;

struct VertexData
{
	glm::vec2	m_vertex;
	glm::vec3	m_color;
	glm::vec2	m_texCoord;
};

struct SpriteInfo
{
	std::array<VertexData, 4>	m_verticies;
	std::string					m_texturePath;
};

struct ImGuiInitInfo
{
	uint32_t			m_apiVersion;
	VkInstance			m_instance;
	VkPhysicalDevice	m_physicalDevice;
	VkDevice			m_device;
	uint32_t			m_queueFamily;
	VkQueue				m_queue;
	VkDescriptorPool	m_descriptorPool;
	VkRenderPass		m_renderPass;
	uint32_t			m_minImageCount;
	uint32_t			m_imageCount;
};

class RendererManager
{
public:
	void addSpriteToDrawList(const SpriteInfo& spriteInfo)
	{
		m_sprites.push_back(spriteInfo);
	}

	//-- We need this function only to avoid ImGuiSystem knowing anything about vulkan device
	const ImGuiInitInfo& imguiInfo() { return m_imGuiInitInfo; }

private:
	friend class RendererSystem;

	//-- User notation object
	std::vector<SpriteInfo>				m_sprites;
	ImGuiInitInfo						m_imGuiInitInfo;
};