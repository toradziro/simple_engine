#include "renderer.h"

#include <application/engine.h>
#include <application/managers/window_manager.h>


#include <filesystem>
#include <iostream>
#include <algorithm>
#include <ranges>

RendererSystem::RendererSystem(EngineContext& context)
	: m_engineContext(context)
{
	try
	{
		m_device.init(m_engineContext.m_managerHolder.getManager<WindowManager>().window());
		m_texureCache = std::make_unique<TextureCache>(m_device);
		m_batchDrawer = std::make_unique<BatchDrawer>(m_device);
	}
	catch (const std::exception& e)
	{
		std::cout << "Error during initialization: " << e.what() << std::endl;
		throw;
	}
}

RendererSystem::~RendererSystem()
{
	m_device.waitGraphicIdle();
	m_batchDrawer.reset();
}

void RendererSystem::update(float dt)
{
	beginFrame(dt);
	endFrame();
}

void RendererSystem::beginFrame(float dt)
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

void RendererSystem::endFrame()
{
	try
	{
		const auto currFrameIndex = m_device.currFrame();

		batchSprites();
		//-- Batch drawer will call device drawing
		m_batchDrawer->draw(m_batchedByTextureSprites);

		//-- Clear collections, for now rendering has no any caches
		m_batchedByTextureSprites.clear();

		m_engineContext.m_managerHolder.getManager<RendererManager>().m_sprites.clear();
	}
	catch (const std::exception& e)
	{
		std::cout << "Error during EndFrame: " << e.what() << std::endl;
		throw;
	}
}

void RendererSystem::batchSprites()
{
	auto& sprites = m_engineContext.m_managerHolder.getManager<RendererManager>().m_sprites;
	if (sprites.empty())
	{
		return;
	}

	//-- Group by textures
	auto batches = sprites | std::views::chunk_by([](const auto& lhs, const auto& rhs)
		{
			return lhs.m_texturePath == rhs.m_texturePath;
		});

	//-- Create batches
	for (const auto& batch : batches)
	{
		//-- Texture path from the first element of group
		const std::string& texturePath = batch.front().m_texturePath;
		VulkanTexture* texture = m_texureCache->loadTexture(texturePath);

		//-- Gather all vertices
		std::vector<std::array<VertexData, 4>> batchedVertices;
		batchedVertices.reserve(std::ranges::distance(batch));

		for (const auto& sprite : batch)
		{
			batchedVertices.push_back(sprite.m_verticies);
		}

		//-- Finally - we got the batch
		TexuredSpriteBatch spriteBatch = { std::move(batchedVertices), texture, static_cast<uint32_t>(batch.size()) };
		m_batchedByTextureSprites.emplace_back(std::move(spriteBatch));
	}
}

VulkanTexture* TextureCache::loadTexture(const std::string& texturePath)
{
	if (m_texturesMap.count(texturePath))
	{
		return m_texturesMap[texturePath].get();
	}

	auto curr_path = std::filesystem::current_path();
	auto root_path = curr_path.parent_path();
	auto full_path = root_path / texturePath;

	auto res = m_texturesMap.insert({ texturePath, std::make_unique<VulkanTexture>(full_path.string(), &m_graphicDevice) });

	return res.first->second.get();
}

BatchDrawer::BatchDrawer(VkGraphicDevice& graphicDevice) : m_graphicDevice(graphicDevice)
{
	m_vertexBuffersToFrames.resize(m_graphicDevice.maxFrames());
	m_indexBuffersToFrames.resize(m_graphicDevice.maxFrames());
}

BatchDrawer::~BatchDrawer()
{
	for (uint32_t i = 0; i < m_graphicDevice.maxFrames(); ++i)
	{
		clearBuffers(static_cast<uint8_t>(i));
	}
}

void BatchDrawer::draw(const std::vector<TexuredSpriteBatch>& spriteBatches)
{
	const auto currFrameIndex = m_graphicDevice.currFrame();

	clearBuffers(currFrameIndex);

	//-- Getting batches for current frame
	auto& currentTexturedGeometryBatch = m_vertexBuffersToFrames[currFrameIndex];
	auto& currentIndexBatch = m_indexBuffersToFrames[currFrameIndex];

	currentTexturedGeometryBatch.reserve(spriteBatches.size());
	currentIndexBatch.reserve(spriteBatches.size());

	//-- Transform geometry data into vulkan inner buffers format and preparing indecies for that
	for (auto& batch : spriteBatches)
	{
		TexturedGeometry texturedGeometry = {
			.m_memory = m_graphicDevice.createCombinedVertexBuffer(batch.m_geometryBatch),
			.m_texture = batch.m_texture,
			.m_spritesCount = batch.m_spritesCount
		};

		currentTexturedGeometryBatch.emplace_back(std::move(texturedGeometry));
		currentIndexBatch.push_back(m_graphicDevice.createIndexBuffer(batch.m_spritesCount));
	}

	//-- Drawind self processed here
	m_graphicDevice.endFrame(currentTexturedGeometryBatch, currentIndexBatch);
}

void BatchDrawer::clearBuffers(uint8_t frameIndex)
{
	auto& currentTexturedGeometryBatch = m_vertexBuffersToFrames[frameIndex];
	auto& currentIndexBatch = m_indexBuffersToFrames[frameIndex];

	for (auto& batch : currentTexturedGeometryBatch)
	{
		if (batch.m_memory.m_buffer != VK_NULL_HANDLE)
		{
			m_graphicDevice.clearBuffer(batch.m_memory);
		}
	}
	for (auto& indexBatch : currentIndexBatch)
	{
		if (indexBatch.m_buffer != VK_NULL_HANDLE)
		{
			m_graphicDevice.clearBuffer(indexBatch);
		}
	}

	currentTexturedGeometryBatch.clear();
	currentIndexBatch.clear();
}
