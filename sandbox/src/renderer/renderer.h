#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT_ON_RESULT

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <vector>
#include <array>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

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
};

struct Queues
{
	vk::Queue	m_graphicQueue;
	vk::Queue	m_presentationQueue;
};

struct SwapchainImage
{
	vk::Image		m_image		= VK_NULL_HANDLE;
	vk::ImageView	m_imageView	= VK_NULL_HANDLE;
};

struct SwapChainDetails
{
	//-- Surface props such as size and buffer images count
	vk::SurfaceCapabilitiesKHR			m_surfaceCapabilities ;
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

class Renderer
{
public:
	~Renderer();

	void init(GLFWwindow* window);
	void shutdown();
	void update(float /*dt*/);
	void resizedWindow() { m_framebufferResized = true; }

private:
	void	drawFrame(float /*dt*/);
	void	updateUniformBuffer();

	uint32_t	findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

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
	void	createVertexBuffer();
	void	createIndexBuffer();
	void	createUniformBuffers();
	void	createCommandBuffer();
	void	createSyncObjects();
	void	setupPhysicalDevice(); 

	void	recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);

	void	checkExtentionsSupport(const std::vector<const char*>& instanceExtentionsAppNeed) const;
	void	checkValidationLayerSupport(const std::vector<const char*>& validationLayerAppNeed) const;
	bool	checkDeviceExtentionsSupport(const std::vector<const char*>& deviceExts, vk::PhysicalDevice physicalDevice) const;
	
	auto	checkIfPhysicalDeviceSuitable(vk::PhysicalDevice device) const -> PhysicalDeviceData;
	auto	checkQueueFamilies(vk::PhysicalDevice device) const -> QueueFamilies;
	auto	swapchainDetails(vk::PhysicalDevice device) const -> SwapChainDetails;

	auto	chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& supportedFormats) -> vk::SurfaceFormatKHR;
	auto	choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) -> vk::PresentModeKHR;
	auto	chooseSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities) -> vk::Extent2D;

	auto	createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) -> vk::ImageView;

	void	createBuffer(vk::DeviceSize size
		, vk::BufferUsageFlags usageFlags
		, vk::MemoryPropertyFlags memPropFlags
		, vk::Buffer& buffer
		, vk::DeviceMemory& deviceMemory);
	void	copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

	void	cleanupSwapchain();

private:
	const std::vector<const char*> C_DEVICE_EXTENTIONS
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

private:
	PhysicalDeviceData				m_physicalDeviceData;
	std::vector<SwapchainImage>		m_swapchainImages;
	Queues							m_queues;
	UniformBufferObject				m_modelViewProj;
	GLFWwindow*						m_window = nullptr;
	vk::Instance					m_vkInstance;
	vk::PhysicalDevice				m_physicalDevice;
	vk::Device						m_logicalDevice;
	vk::SurfaceKHR					m_surface;
	vk::SwapchainKHR				m_swapchain;
	vk::ShaderModule				m_vertexShaderModule;
	vk::ShaderModule				m_fragmentShaderModule;
	vk::RenderPass					m_renderPass;
	vk::DescriptorSetLayout			m_descriptorSetLayout;
	vk::PipelineLayout				m_pipelineLayout;
	vk::Pipeline					m_graphicsPipeline;

	vk::Buffer						m_vertexBuffer;
	vk::DeviceMemory				m_vertexBufferMem;
	vk::Buffer						m_indexBuffer;
	vk::DeviceMemory				m_indexBufferMem;
	std::vector<vk::Buffer>			m_unifoirmBuffers;
	std::vector<vk::DeviceMemory>	m_unifoirmBuffersMemory;
	std::vector<void*>				m_uniformBuffersMapped;

	vk::CommandPool					m_commandPool;
	std::vector<vk::CommandBuffer>	m_commandBuffers;
	std::vector<vk::Framebuffer>	m_swapChainFramebuffers;

	vk::SurfaceFormatKHR			m_surfaceFormat;
	vk::Extent2D					m_imageExtent;
	vk::PresentModeKHR				m_presentMode;

	std::vector<vk::Semaphore>		m_imageAvailableSemaphores;
	std::vector<vk::Semaphore>		m_renderFinishedSemaphores;
	std::vector<vk::Fence>			m_inFlightFences;

	const std::vector<VertexData>	m_vertices = {
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
		{{-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}}
	};

	const std::vector<uint16_t>		m_indicies = { 0, 1, 2, 2, 3, 0 };

	uint32_t						m_currFrame = 0;

	bool							m_framebufferResized = false;
#ifdef NDEBUG
	const bool			enableValidationLayers = false;
#else
	const bool			enableValidationLayers = true;
#endif
};