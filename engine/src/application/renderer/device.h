#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <vector>
#include <array>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <iostream>
#include <format>
#include <algorithm>
#include <filesystem>
#include <cstdio>
#include <fstream>
#include <shaderc/shaderc.hpp>
#include <cstdlib>
#include <print>
#include <numeric>

#include <application/managers/renderer_manager.h>
#include <application/editor/imgui_integration.h>

constexpr int C_MAX_FRAMES_IN_FLIGHT = 2;

struct EngineContext;

//-------------------------------------------------------------------------------------------------
struct VertexData
{
	glm::vec4 m_vertex;
	glm::vec3 m_color;
	glm::vec2 m_texCoord;
};

//-------------------------------------------------------------------------------------------------
struct QueueFamilies
{
	int m_graphicQueue = -1;
	int m_presentationQueue = -1;

	bool isValid() const
	{
		return m_graphicQueue >= 0
		       && m_presentationQueue >= 0;
	}
};

//-------------------------------------------------------------------------------------------------
struct Queues
{
	vk::Queue m_graphicQueue;
	vk::Queue m_presentationQueue;
};

//-------------------------------------------------------------------------------------------------
struct SwapchainImage
{
	vk::Image     m_image = VK_NULL_HANDLE;
	vk::ImageView m_imageView = VK_NULL_HANDLE;
};

//-------------------------------------------------------------------------------------------------
struct SwapChainDetails
{
	//-- Surface props such as size and buffer images count
	vk::SurfaceCapabilitiesKHR m_surfaceCapabilities;
	//-- Format such as RGBA8
	std::vector<vk::SurfaceFormatKHR> m_surfaceSupportedFormats;
	//-- Supported presentation modes to choose
	std::vector<vk::PresentModeKHR> m_presentMode;
};

//-------------------------------------------------------------------------------------------------
struct PhysicalDeviceData
{
	int              m_score = 0;
	QueueFamilies    m_queueFamilies;
	SwapChainDetails m_swapchainDetails;
};

//-------------------------------------------------------------------------------------------------
struct UniformBufferObject
{
	glm::mat4 m_view;
	glm::mat4 m_proj;
};

//-------------------------------------------------------------------------------------------------
struct VulkanBufferMemory
{
	vk::Buffer       m_buffer;
	vk::DeviceMemory m_bufferMem;
};

//-------------------------------------------------------------------------------------------------
struct TexturedGeometry
{
	VulkanBufferMemory m_memory;
	vk::DescriptorSet  m_textureDescriptorSet;
	uint32_t           m_spritesCount;
};

using TexturedGeometryBatch = std::vector<TexturedGeometry>;
using BatchIndecies = std::vector<VulkanBufferMemory>;

//-------------------------------------------------------------------------------------------------
class VkGraphicDevice
{
public:
	explicit VkGraphicDevice(std::shared_ptr<EngineContext> context) : m_engineContext(context) {}
	~VkGraphicDevice();

	void init(GLFWwindow* window);
	void shutdown();
	void resizedWindow();
	void beginFrame(float /*dt*/);
	void endFrame(const TexturedGeometryBatch& geometryBatch, const BatchIndecies& indicesBatch);
	auto createIndexBuffer(uint16_t spriteCount) -> VulkanBufferMemory;
	void clearBuffer(VulkanBufferMemory memory);
	VulkanBufferMemory createCombinedVertexBuffer(const std::vector<std::array<VertexData, 4>>& sprites);
	uint8_t maxFrames() const;
	uint8_t currFrame() const;
	void waitGraphicIdle();
	void updateUniformBuffer();
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
	vk::Device& getLogicalDevice();
	void createVkInstance();
	void createLogicalDevice();
	void createSurface();
	void recreateSwapChain();
	void createSwapchain();
	void createShaderModule();
	void createDescriptorSetLayout();
	void createPipeline();
	void createRenderPass();
	void createFramebuffer();
	void createCommandPool();
	void createTextureSampler();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorsSets();
	void freeDescriptorSetFromPool(vk::DescriptorSet& descriptorSet);
	void createCommandBuffer();
	void createSyncObjects();
	void setupPhysicalDevice();
	vk::DescriptorSet createTextureDescriptorSet(vk::Image& image, vk::ImageView& imageView);
	void recordCommandBuffer(vk::CommandBuffer              commandBuffer
	                         , uint32_t                     imageIndex
	                         , const TexturedGeometryBatch& geometryBatch
	                         , const BatchIndecies&         indicesBatch);
	
	//-- const char* here because glfw returns const char** as extentions list
	void checkExtensionsSupport(const std::vector<const char*>& instanceExtentionsAppNeed) const;
	void checkValidationLayerSupport(const std::vector<const char*>& validationLayerAppNeed) const;
	bool checkDeviceExtensionsSupport(const std::vector<const char*>& deviceExtentions
	                                  , vk::PhysicalDevice            physicalDevice) const;
	PhysicalDeviceData checkIfPhysicalDeviceSuitable(vk::PhysicalDevice device) const;
	QueueFamilies checkQueueFamilies(vk::PhysicalDevice device) const;
	SwapChainDetails swapchainDetails(vk::PhysicalDevice device) const;
	vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& supportedFormats);
	vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
	vk::Extent2D chooseSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
	vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);
	void createBuffer(vk::DeviceSize            size
	                  , vk::BufferUsageFlags    usageFlags
	                  , vk::MemoryPropertyFlags memPropFlags
	                  , vk::Buffer&             buffer
	                  , vk::DeviceMemory&       deviceMemory);
	void transitionImage(vk::Image         image
	                     , vk::Format      format
	                     , vk::ImageLayout oldLayout
	                     , vk::ImageLayout newLayout);
	void copyBufferToImage(vk::Buffer  buffer
	                       , vk::Image image
	                       , uint32_t  width
	                       , uint32_t  height);
	void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
	vk::CommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommand(vk::CommandBuffer commandBuffer);
	void cleanupSwapchain();
	void setMaxTextures(uint32_t maxPossibleTextures);

	//-- Info getters
	uint32_t apiVersion() const { return m_apiVersion; }
	uint32_t getGraphicQueueFamily() const { return m_physicalDeviceData.m_queueFamilies.m_graphicQueue; }
	uint32_t minImageCount() const { return m_swapchainImages.size(); }
	uint32_t imageCount() const { return m_swapchainImages.size(); }
	vk::Instance instance() const { return m_vkInstance; }
	vk::PhysicalDevice physicalDevice() const { return m_physicalDevice; }
	vk::Device device() const { return m_logicalDevice; }
	vk::Queue graphicQueue() const { return m_queues.m_graphicQueue; }
	vk::DescriptorPool descriptorPool() const { return m_descriptorPool; }
	vk::RenderPass renderPass() const { return m_renderPass; }

	void setImGuiDrawCallbacks(std::vector<RendererManager::ImGuiDrawCallback>& imGuiDrawCallbacks)
	{
		m_imGuiDrawCallbacks = imGuiDrawCallbacks;
	}

private:
	const std::vector<const char*> C_DEVICE_EXTENSIONS
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
		,
	};

private:
	std::shared_ptr<EngineContext> m_engineContext;

	PhysicalDeviceData             m_physicalDeviceData;
	std::vector<SwapchainImage>    m_swapchainImages;
	Queues                         m_queues;
	GLFWwindow*                    m_window = nullptr;
	vk::Instance                   m_vkInstance;
	vk::PhysicalDevice             m_physicalDevice;
	vk::Device                     m_logicalDevice;
	vk::SurfaceKHR                 m_surface;
	vk::SwapchainKHR               m_swapchain;
	vk::ShaderModule               m_vertexShaderModule;
	vk::ShaderModule               m_fragmentShaderModule;
	vk::RenderPass                 m_renderPass;
	vk::DescriptorSetLayout        m_uniformsSetLayout;
	vk::DescriptorSetLayout        m_texturesSetLayout;
	vk::DescriptorPool             m_descriptorPool;
	std::vector<vk::DescriptorSet> m_descriptorSets;
	vk::PipelineLayout             m_pipelineLayout;
	vk::Pipeline                   m_graphicsPipeline;

	std::vector<vk::Buffer>       m_uniformBuffers;
	std::vector<vk::DeviceMemory> m_uniformBuffersMemory;
	std::vector<void*>            m_uniformBuffersMapped;

	vk::Sampler m_textureSampler;

	vk::CommandPool                m_commandPool;
	std::vector<vk::CommandBuffer> m_commandBuffers;
	std::vector<vk::Framebuffer>   m_swapChainFramebuffers;

	vk::SurfaceFormatKHR m_surfaceFormat;
	vk::Extent2D         m_imageExtent;
	vk::PresentModeKHR   m_presentMode = {};

	std::vector<vk::Semaphore> m_imageAvailableSemaphores;
	std::vector<vk::Semaphore> m_renderFinishedSemaphores;
	std::vector<vk::Fence>     m_inFlightFences;

	ImGuiIntegration                                m_imGuiIntegration;
	std::vector<RendererManager::ImGuiDrawCallback> m_imGuiDrawCallbacks;

	uint32_t m_apiVersion = 0;
	uint32_t m_currFrame = 0;
	uint32_t m_currImageIndex = 0;
	uint32_t m_maxTextures = 200;

	bool m_framebufferResized = false;
#ifdef NDEBUG
	const bool						m_enableValidationLayers = false;
#else
	const bool m_enableValidationLayers = true;
#endif
};
