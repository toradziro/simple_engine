#include "renderer.h"
#include <filesystem>
#include <iostream>

Renderer::Renderer(GLFWwindow* window)
{
	try
	{
		m_device.init(window);

		m_vertexBuffersToFrames.resize(m_device.maxFrames());
		m_indexBuffersToFrames.resize(m_device.maxFrames());
	}
	catch (const std::exception& e)
	{
		std::cout << "Error during initialization: " << e.what() << std::endl;
		throw;
	}
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
	try
	{
		m_device.beginFrame(dt);
	}
	catch (const std::exception& e)
	{
		std::cout << "Error during BeginFrame: " << e.what() << std::endl;
		throw;
	}
}

void Renderer::endFrame()
{
	try
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
	catch (const std::exception& e)
	{
		std::cout << "Error during EndFrame: " << e.what() << std::endl;
		throw;
	}
}

void Renderer::drawSprite(const SpriteInfo& spriteInfo)
{
	//-- Create vertex buffer here
	m_sprites.push_back(spriteInfo.m_verticies);
}

void Renderer::setTexture(const std::string& path)
{
	try
	{
		auto curr_path = std::filesystem::current_path();
		auto&& root_path = curr_path.parent_path();
		auto&& full_cat_path = root_path / path;
		auto texture = std::make_unique<VulkanTexture>(full_cat_path.string(), &m_device);

		m_device.setTexture(std::move(texture));
	}
	catch (const std::exception& e)
	{
		std::cout << "Error during Renderer::setTexture: " << e.what() << std::endl;
		throw;
	}
}
