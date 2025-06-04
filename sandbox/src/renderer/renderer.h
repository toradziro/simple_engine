#pragma once

#include "device.h"

struct SpriteInfo
{
	std::array<VertexData, 4>	m_verticies;
};

class Renderer
{
public:
	Renderer(GLFWwindow* window);
	~Renderer();

	void beginFrame(float dt);
	void endFrame();
	void drawSprite(const SpriteInfo& spriteInfo);
	void resizedWindow() { m_device.resizedWindow(); }

	void setTexture(const std::string& path);

private:
	//void clearVertexBuffers();

private:
	VkGraphicDevice							m_device;
	//std::vector<VulkanBufferMemory>			m_vertexBuffers;
	std::vector<std::array<VertexData, 4>>	m_sprites; 
	std::vector<VulkanBufferMemory>			m_vertexBuffersToFrames;
	std::vector<VulkanBufferMemory>			m_indexBuffersToFrames;
};