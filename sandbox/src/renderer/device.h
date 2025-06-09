#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <vector>
#include <array>
#include <memory>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "texture.h"

struct QueueFamilies
{
	int	m_graphicQueue = -1;
	int	m_presentationQueue = -1;

	bool isValid() const
	{
		return m_graphicQueue >= 0
			&& m_presentationQueue >= 0;
	}
};

struct VertexData
{
	glm::vec2	m_vertex;
	glm::vec3	m_color;
	glm::vec2	m_texCoord;
};

struct Queues
{
	vk::Queue	m_graphicQueue;
	vk::Queue	m_presentationQueue;
};

struct SwapchainImage
{
	vk::Image		m_image = VK_NULL_HANDLE;
	vk::ImageView	m_imageView = VK_NULL_HANDLE;
};

struct SwapChainDetails
{
	//-- Surface props such as size and buffer images count
	vk::SurfaceCapabilitiesKHR			m_surfaceCapabilities;
	//-- Format such as RGBA8
	std::vector<vk::SurfaceFormatKHR>	m_surfaceSupportedFormats;
	//-- Supported presentation modes to choose
	std::vector<vk::PresentModeKHR>		m_presentMode;
};

struct PhysicalDeviceData
{
	int					m_score = 0;
	QueueFamilies		m_queueFamilies;
	SwapChainDetails	m_swapchainDetails;
};

struct UniformBufferObject
{
	glm::mat4	m_model;
	glm::mat4	m_view;
	glm::mat4	m_proj;
};

struct VulkanBufferMemory
{
	vk::Buffer			m_buffer;
	vk::DeviceMemory	m_bufferMem;
};

struct TexturedGeometry
{
	VulkanBufferMemory	m_memory;
	VulkanTexture*		m_texture;
	uint32_t			m_spritesCount;
};

using TexturedGeometryBatch = std::vector<TexturedGeometry>;
using BatchIndecies = std::vector<VulkanBufferMemory>;

class VkGraphicDevice
{
public:
	~VkGraphicDevice();

	void init(GLFWwindow* window);
	void shutdown();
	void resizedWindow()
	{
		m_framebufferResized = true;
	}

	void	beginFrame(float /*dt*/);
	void	endFrame(const TexturedGeometryBatch& geometryBatch, const BatchIndecies& indicesBatch);

	auto	createIndexBuffer(uint16_t spriteCount) -> VulkanBufferMemory;
	void	clearBuffer(VulkanBufferMemory memory);

	void	setTexture(std::unique_ptr<VulkanTexture>&& texture) { m_texture = std::move(texture); }

	VulkanBufferMemory	createCombinedVertexBuffer(const std::vector<std::array<VertexData, 4>>& sprites);
	
	uint8_t	maxFrames() const;
	uint8_t	currFrame() const;

	void	waitGraphicIdle();

	void	updateUniformBuffer();

	uint32_t	findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

	vk::Device&	getLogicalDevice() { return m_logicalDevice; }

	void	createVkInstance();
	void	createLogicalDevice();
	void	createSurface();
	void	recreateSwapChain();
	void	createSwapchain();
	void	createShaderModule();
	void	createDescriptorSetLayout();
	void	createPipeline();
	void	createRenderPass();
	void	createFramebuffer();
	void	createCommandPool();
	void	createTextureSampler();
	void	createUniformBuffers();
	void	createDescriptorPool();
	void	createDescriptorsSets();
	void	freeDescriptorSetFromPool(vk::DescriptorSet& descriptorSet);
	void	createCommandBuffer();
	void	createSyncObjects();
	void	setupPhysicalDevice();

	vk::DescriptorSet	createTextureDescriptorSet(vk::Image& image, vk::ImageView& imageView);

	void	recordCommandBuffer(vk::CommandBuffer commandBuffer
		, uint32_t imageIndex
		, const TexturedGeometryBatch& geometryBatch
		, const BatchIndecies& indicesBatch);

	void	checkExtentionsSupport(const std::vector<const char*>& instanceExtentionsAppNeed) const;
	void	checkValidationLayerSupport(const std::vector<const char*>& validationLayerAppNeed) const;
	[[nodiscard]] bool	checkDeviceExtentionsSupport(const std::vector<const char*>& deviceExtentions, vk::PhysicalDevice physicalDevice) const;

	[[nodiscard]] auto	checkIfPhysicalDeviceSuitable(vk::PhysicalDevice device) const->PhysicalDeviceData;
	[[nodiscard]] auto	checkQueueFamilies(vk::PhysicalDevice device) const->QueueFamilies;
	[[nodiscard]] auto	swapchainDetails(vk::PhysicalDevice device) const->SwapChainDetails;

	auto	chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& supportedFormats) -> vk::SurfaceFormatKHR;
	auto	choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) -> vk::PresentModeKHR;
	auto	chooseSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities) -> vk::Extent2D;

	auto	createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) -> vk::ImageView;

	void	createBuffer(vk::DeviceSize size
		, vk::BufferUsageFlags usageFlags
		, vk::MemoryPropertyFlags memPropFlags
		, vk::Buffer& buffer
		, vk::DeviceMemory& deviceMemory);

	void	transitionImage(vk::Image image
		, vk::Format format
		, vk::ImageLayout oldLayout
		, vk::ImageLayout newLayout);

	void	copyBufferToImage(vk::Buffer buffer
		, vk::Image image
		, uint32_t width
		, uint32_t height);

	void	copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

	auto	beginSingleTimeCommands() -> vk::CommandBuffer;
	void	endSingleTimeCommand(vk::CommandBuffer commandBuffer);

	void	cleanupSwapchain();

	void	setMaxTextures(uint32_t maxPossibleTextures) { m_maxTextures = maxPossibleTextures; }

private:
	const std::vector<const char*> C_DEVICE_EXTENTIONS
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

private:
	PhysicalDeviceData				m_physicalDeviceData;
	std::vector<SwapchainImage>		m_swapchainImages;
	Queues							m_queues;
	GLFWwindow*						m_window = nullptr;
	vk::Instance					m_vkInstance;
	vk::PhysicalDevice				m_physicalDevice;
	vk::Device						m_logicalDevice;
	vk::SurfaceKHR					m_surface;
	vk::SwapchainKHR				m_swapchain;
	vk::ShaderModule				m_vertexShaderModule;
	vk::ShaderModule				m_fragmentShaderModule;
	vk::RenderPass					m_renderPass;
	vk::DescriptorSetLayout			m_uniformsSetLayout;
	vk::DescriptorSetLayout			m_texturesSetLayout;
	vk::DescriptorPool				m_descriptorPool;
	std::vector<vk::DescriptorSet>	m_descriptorSets;
	vk::PipelineLayout				m_pipelineLayout;
	vk::Pipeline					m_graphicsPipeline;

	std::vector<vk::Buffer>			m_uniformBuffers;
	std::vector<vk::DeviceMemory>	m_uniformBuffersMemory;
	std::vector<void*>				m_uniformBuffersMapped;

	std::unique_ptr<VulkanTexture>	m_texture;
	vk::Sampler						m_textureSampler;

	vk::CommandPool					m_commandPool;
	std::vector<vk::CommandBuffer>	m_commandBuffers;
	std::vector<vk::Framebuffer>	m_swapChainFramebuffers;

	vk::SurfaceFormatKHR			m_surfaceFormat;
	vk::Extent2D					m_imageExtent;
	vk::PresentModeKHR				m_presentMode = {};

	std::vector<vk::Semaphore>		m_imageAvailableSemaphores;
	std::vector<vk::Semaphore>		m_renderFinishedSemaphores;
	std::vector<vk::Fence>			m_inFlightFences;

	uint32_t						m_currFrame = 0;
	uint32_t						m_currImageIndex = 0;
	uint32_t						m_maxTextures = 200;

	bool							m_framebufferResized = false;
#ifdef NDEBUG
	const bool						m_enableValidationLayers = false;
#else
	const bool						m_enableValidationLayers = true;
#endif
};