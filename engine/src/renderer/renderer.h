#pragma once

#include "device.h"
#include <unordered_map>
#include <memory>

#include <application/managers/renderer_manager.h>
#include <application/managers/events/event_interface.h>

struct EngineContext;

struct TexuredSpriteBatch
{
	std::vector<std::array<VertexData, 4>>	m_geometryBatch;
	VulkanTexture*							m_texture;
	uint32_t								m_spritesCount;
};

class VulkanTexture;

class TextureCache
{
public:
	TextureCache(VkGraphicDevice& graphicDevice) : m_graphicDevice(graphicDevice) {}

	VulkanTexture*	loadTexture(const std::string& texturePath);

private:
	using TextureMap = std::unordered_map<std::string, std::unique_ptr<VulkanTexture>>;

	TextureMap			m_texturesMap;
	VkGraphicDevice&	m_graphicDevice;
};

class BatchDrawer
{
public:
	BatchDrawer(VkGraphicDevice& graphicDevice);
	~BatchDrawer();

	void	draw(const std::vector<TexuredSpriteBatch>& spriteBatches);

private:
	void	clearBuffers(uint8_t frameIndex);

	VkGraphicDevice&					m_graphicDevice;
	//-- Transformed to device notation data
	std::vector<TexturedGeometryBatch>	m_vertexBuffersToFrames;
	std::vector<BatchIndecies>			m_indexBuffersToFrames;
};

class RendererSystem
{
public:
	RendererSystem(EngineContext& context);
	~RendererSystem();

	void update(float dt);
	void onEvent(Event& event);
	void beginFrame(float dt);
	void endFrame();
	void resizedWindow() { m_device.resizedWindow(); }

private:
	void batchSprites();

private:
	EngineContext&						m_engineContext;

	VkGraphicDevice						m_device;

	std::unique_ptr<TextureCache>		m_texureCache;
	std::unique_ptr<BatchDrawer>		m_batchDrawer;

	//-- Transfromed to batches user's data
	std::vector<TexuredSpriteBatch>		m_batchedByTextureSprites;
};