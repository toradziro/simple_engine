#pragma once

#include "device.h"
#include <unordered_map>
#include <memory>

struct SpriteInfo
{
	std::array<VertexData, 4>	m_verticies;
	std::string					m_texturePath;
};

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

class Renderer
{
public:
	Renderer(GLFWwindow* window);
	Renderer(Renderer&& renderer) = default;
	~Renderer();

	void update(float dt) {}
	void beginFrame(float dt);
	void endFrame();
	void drawSprite(const SpriteInfo& spriteInfo);
	void resizedWindow() { m_device.resizedWindow(); }

private:
	void batchSprites();

private:
	VkGraphicDevice						m_device;

	std::unique_ptr<TextureCache>		m_texureCache;
	std::unique_ptr<BatchDrawer>		m_batchDrawer;

	//-- User notation object
	std::vector<SpriteInfo>				m_sprites;
	//-- Transfromed to batches user's data
	std::vector<TexuredSpriteBatch>		m_batchedByTextureSprites;
};