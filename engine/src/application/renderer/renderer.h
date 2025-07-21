#pragma once

#include <absl/container/flat_hash_map.h>
#include <memory>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <ranges>
#include <print>
#include <format>
#include <vector>
#include <vulkan/vulkan.hpp>

#include <application/renderer/texture.h>
#include <application/managers/renderer_manager.h>
#include <application/renderer/device.h>

struct Event;
struct EngineContext;

//-------------------------------------------------------------------------------------------------
constexpr std::array<VertexData, 4> C_QUAD_BASIC_DATA =
{
	VertexData{ { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }
	, VertexData{ { 0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } }
	, VertexData{ { 0.5f, 0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } }
	, VertexData{ { -0.5f, 0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } }
};

//-------------------------------------------------------------------------------------------------
struct TexuredSpriteBatch
{
	std::vector<std::array<VertexData, 4>> m_geometryBatch;
	VulkanTexture*                         m_texture;
	uint32_t                               m_spritesCount;
};

//-------------------------------------------------------------------------------------------------
class TextureCache
{
public:
	TextureCache(VkGraphicDevice& graphicDevice, EngineContext& context) : m_graphicDevice(graphicDevice)
	                                                                     , m_engineContext(context) {}

	VulkanTexture* loadTexture(const std::string& texturePath);

private:
	using TextureMap = absl::flat_hash_map<std::string, std::unique_ptr<VulkanTexture>>;

	TextureMap       m_texturesMap;
	VkGraphicDevice& m_graphicDevice;
	EngineContext&   m_engineContext;
};

class BatchDrawer
{
public:
	BatchDrawer(VkGraphicDevice& graphicDevice);
	~BatchDrawer();

	void draw(const std::vector<TexuredSpriteBatch>& spriteBatches);

private:
	//-------------------------------------------------------------------------------------------------
	void clearBuffers(uint8_t frameIndex);

	VkGraphicDevice& m_graphicDevice;
	//-- Transformed to device notation data
	std::vector<TexturedGeometryBatch> m_vertexBuffersToFrames;
	std::vector<BatchIndecies>         m_indexBuffersToFrames;
};

//-------------------------------------------------------------------------------------------------
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
	EngineContext& m_engineContext;

	VkGraphicDevice m_device;

	std::unique_ptr<TextureCache> m_texureCache;
	std::unique_ptr<BatchDrawer>  m_batchDrawer;

	//-- Transfromed to batches user's data
	std::vector<TexuredSpriteBatch> m_batchedByTextureSprites;
};
