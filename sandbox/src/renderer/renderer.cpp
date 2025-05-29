#include "renderer.h"

Renderer::Renderer(GLFWwindow* window)
{
	m_device.init(window);
	m_vertexBuffersToFrames.resize(m_device.maxFrames());
	m_indexBuffersToFrames.resize(m_device.maxFrames());
}

Renderer::~Renderer()
{
	m_device.waitGraphicIdle();
	for (auto& buff : m_vertexBuffersToFrames)
	{
		if (buff.m_Buffer != VK_NULL_HANDLE)
		{
			m_device.clearBuffer(buff);
		}
	}
	for (auto& buff : m_indexBuffersToFrames)
	{
		if (buff.m_Buffer != VK_NULL_HANDLE)
		{
			m_device.clearBuffer(buff);
		}
	}
}

void Renderer::beginFrame(float dt)
{
	m_device.beginFrame(dt);
}

void Renderer::endFrame()
{
	const auto currFrameIndex = m_device.currFrame();

	if (m_vertexBuffersToFrames[currFrameIndex].m_Buffer != VK_NULL_HANDLE)
	{
		m_device.clearBuffer(m_vertexBuffersToFrames[currFrameIndex]);
		m_device.clearBuffer(m_indexBuffersToFrames[currFrameIndex]);
	}

	m_vertexBuffersToFrames[currFrameIndex] = m_device.createCombinedVertexBuffer(m_sprites);
	m_indexBuffersToFrames[currFrameIndex] = m_device.createIndexBuffer(m_sprites.size());

	m_device.endFrame(m_vertexBuffersToFrames[currFrameIndex]
		, m_indexBuffersToFrames[currFrameIndex]
		, m_sprites.size());
	m_sprites.clear();
}

void Renderer::drawSprite(const SpriteInfo& spriteInfo)
{
	//-- Create vertex buffer here
	m_sprites.push_back(spriteInfo.m_verticies);
}
