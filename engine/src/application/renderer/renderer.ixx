module;

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>

export module renderer_system;

import <absl/container/flat_hash_map.h>;
import <memory>;
import <filesystem>;
import <iostream>;
import <algorithm>;
import <ranges>;
import <print>;
import <format>;
import <vector>;
import <vulkan/vulkan.hpp>;

import graphic_device;
import renderer_manager;
import event_interface;
import window_manager;
import engine_context;
import vulkan_texture;
import virtual_fs;
import engine_assert;

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
	//-------------------------------------------------------------------------------------------------
	TextureCache(VkGraphicDevice& graphicDevice, EngineContext& context) : m_graphicDevice(graphicDevice)
	                                                                     , m_engineContext(context) {}

	//-------------------------------------------------------------------------------------------------
	VulkanTexture* loadTexture(const std::string& texturePath)
	{
		if (m_texturesMap.count(texturePath))
		{
			return m_texturesMap[texturePath].get();
		}

		auto& vfs = m_engineContext.m_managerHolder.getManager<VirtualFS>();
		engineAssert(vfs.isFileExist(texturePath), "Texture don't exist");
		auto full_path = vfs.virtualToNativePath(texturePath);

		auto res = m_texturesMap.insert({
			texturePath
			, std::make_unique<VulkanTexture>(full_path.string(), &m_graphicDevice)
		});

		return res.first->second.get();
	}

private:
	using TextureMap = absl::flat_hash_map<std::string, std::unique_ptr<VulkanTexture>>;

	TextureMap       m_texturesMap;
	VkGraphicDevice& m_graphicDevice;
	EngineContext&   m_engineContext;
};

class BatchDrawer
{
public:
	//-------------------------------------------------------------------------------------------------
	BatchDrawer(VkGraphicDevice& graphicDevice) : m_graphicDevice(graphicDevice)
	{
		m_vertexBuffersToFrames.resize(m_graphicDevice.maxFrames());
		m_indexBuffersToFrames.resize(m_graphicDevice.maxFrames());
	}

	//-------------------------------------------------------------------------------------------------
	~BatchDrawer()
	{
		for (uint32_t i = 0; i < m_graphicDevice.maxFrames(); ++i)
		{
			clearBuffers(static_cast<uint8_t>(i));
		}
	}

	//-------------------------------------------------------------------------------------------------
	void draw(const std::vector<TexuredSpriteBatch>& spriteBatches)
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
				.m_memory = m_graphicDevice.createCombinedVertexBuffer(batch.m_geometryBatch)
				, .m_textureDescriptorSet = batch.m_texture->getDescriptorSet()
				, .m_spritesCount = batch.m_spritesCount
			};

			currentTexturedGeometryBatch.emplace_back(std::move(texturedGeometry));
			currentIndexBatch.push_back(m_graphicDevice.createIndexBuffer(batch.m_spritesCount));
		}

		//-- Drawind self processed here
		m_graphicDevice.endFrame(currentTexturedGeometryBatch, currentIndexBatch);
	}

private:
	//-------------------------------------------------------------------------------------------------
	void clearBuffers(uint8_t frameIndex)
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

	VkGraphicDevice& m_graphicDevice;
	//-- Transformed to device notation data
	std::vector<TexturedGeometryBatch> m_vertexBuffersToFrames;
	std::vector<BatchIndecies>         m_indexBuffersToFrames;
};

//-------------------------------------------------------------------------------------------------
export class RendererSystem
{
public:
	//-------------------------------------------------------------------------------------------------
	RendererSystem(EngineContext& context)
		: m_engineContext(context)
		, m_device(context)
	{
		m_device.init(m_engineContext.m_managerHolder.getManager<WindowManager>().window());
		m_texureCache = std::make_unique<TextureCache>(m_device, context);
		m_batchDrawer = std::make_unique<BatchDrawer>(m_device);
	}

	//-------------------------------------------------------------------------------------------------
	~RendererSystem()
	{
		m_device.waitGraphicIdle();
		m_batchDrawer.reset();
	}

	//-------------------------------------------------------------------------------------------------
	void update(float dt)
	{
		beginFrame(dt);
		endFrame();
	}

	//-------------------------------------------------------------------------------------------------
	void onEvent(Event& event)
	{
		if (eventTypeCheck<WindowResizeEvent>(event))
		{
			std::println("WindowResizeEvent");

			resizedWindow();
			event.setHandeled();
		}
		//-- Mouse moved check example
		//-- if (eventTypeCheck<MouseMovedEvent>(event))
		//-- {
		//-- 	auto& mouseMoved = event.getUnderlyingEvent<MouseMovedEvent>();
		//-- 	std::println("Mouse moved: {} {}", mouseMoved.m_mouseX, mouseMoved.m_mouseY);
		//-- }
	}

	//-------------------------------------------------------------------------------------------------
	void beginFrame(float dt)
	{
		m_device.beginFrame(dt);
	}

	//-------------------------------------------------------------------------------------------------
	void endFrame()
	{
		const auto currFrameIndex = m_device.currFrame();

		batchSprites();
		//-- Batch drawer will call device drawing
		auto& drawListImGuiUI = m_engineContext.m_managerHolder.getManager<RendererManager>().m_imGuiUpdatesUi;
		m_device.setImGuiDrawCallbacks(drawListImGuiUI);
		m_batchDrawer->draw(m_batchedByTextureSprites);
		m_engineContext.m_managerHolder.getManager<RendererManager>().m_imGuiUpdatesUi.clear();

		//-- Clear collections, for now rendering has no any caches
		m_batchedByTextureSprites.clear();

		m_engineContext.m_managerHolder.getManager<RendererManager>().m_sprites.clear();
	}

	//-------------------------------------------------------------------------------------------------
	void resizedWindow() { m_device.resizedWindow(); }

private:
	//-------------------------------------------------------------------------------------------------
	void batchSprites()
	{
		auto& sprites = m_engineContext.m_managerHolder.getManager<RendererManager>().m_sprites;
		if (sprites.empty())
		{
			return;
		}

		//-- Group by textures
		std::sort(sprites.begin()
		          , sprites.end()
		          , [] (const auto& lhs, const auto& rhs)
		          {
			          return lhs.m_position.z < rhs.m_position.z;
		          });

		auto batches = sprites | std::views::chunk_by([] (const auto& lhs, const auto& rhs)
		{
			return lhs.m_texturePath == rhs.m_texturePath;
		});

		//-- Create batches
		for (const auto& batch : batches)
		{
			//-- Texture path from the first element of group
			const std::string& texturePath = batch.front().m_texturePath;
			VulkanTexture*     texture = m_texureCache->loadTexture(texturePath);

			//-- Gather all vertices
			std::vector<std::array<VertexData, 4>> batchedVertices;
			batchedVertices.reserve(std::distance(batch.begin(), batch.end()));

			//-- Prepare sprite in batch
			for (const auto& sprite : batch)
			{
				//-- TODO: Move this to component method
				glm::mat4 transform = { 1.0f };
				transform = glm::translate(transform, sprite.m_position);
				std::array<VertexData, 4> transformedData = {};

				//-- Here we transfrom from local to world coordinates
				for (int i = 0; i < 4; ++i)
				{
					transformedData[i].m_color = C_QUAD_BASIC_DATA[i].m_color;
					transformedData[i].m_texCoord = C_QUAD_BASIC_DATA[i].m_texCoord;
					transformedData[i].m_vertex = transform * C_QUAD_BASIC_DATA[i].m_vertex;
				}
				batchedVertices.push_back(std::move(transformedData));
			}

			//-- Finally - we got the batch
			TexuredSpriteBatch spriteBatch = {
				std::move(batchedVertices)
				, texture
				, static_cast<uint32_t>(batch.size())
			};
			m_batchedByTextureSprites.emplace_back(std::move(spriteBatch));
		}
	}

private:
	EngineContext& m_engineContext;

	VkGraphicDevice m_device;

	std::unique_ptr<TextureCache> m_texureCache;
	std::unique_ptr<BatchDrawer>  m_batchDrawer;

	//-- Transfromed to batches user's data
	std::vector<TexuredSpriteBatch> m_batchedByTextureSprites;
};
