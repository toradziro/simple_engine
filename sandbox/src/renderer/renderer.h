#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <array>

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

struct Queues
{
	VkQueue	m_graphicQueue;
	VkQueue	m_presentationQueue;
};

struct SwapchainImage
{
	VkImage		m_image		= VK_NULL_HANDLE;
	VkImageView	m_imageView	= VK_NULL_HANDLE;
};

struct SwapChainDetails
{
	//-- Surface props such as size and buffer images count
	VkSurfaceCapabilitiesKHR		m_surfaceCapabilities = {};
	//-- Format such as RGBA8
	std::vector<VkSurfaceFormatKHR>	m_surfaceSupportedFormats;
	//-- Supported presentation modes to choose
	std::vector<VkPresentModeKHR>	m_presentMode;
};

struct PhysicalDeviceData
{
	int					m_score = 0;
	QueueFamilies		m_queueFamilies;
	SwapChainDetails	m_swapchainDetails;
};

class Renderer
{
public:
	constexpr static inline int C_DEVICE_EXTEINTIONS_COUNT = 1;

	~Renderer();

	void init(GLFWwindow* window);
	void shutdown();
	void update(float /*dt*/);

private:

	void	createVkInstance();
	void	createLogicalDevice();
	void	createSurface();
	void	createSwapchain();
	void	createShaderModule();
	void	createPipeline();
	void	createRenderPass();
	void	setupPhysicalDevice();

	void	checkExtentionsSupport(const std::vector<const char*>& instanceExtentionsAppNeed) const;
	void	checkValidationLayerSupport(const std::vector<const char*>& validationLayerAppNeed) const;
	bool	checkDeviceExtentionsSupport(const std::array<const char*, C_DEVICE_EXTEINTIONS_COUNT>& deviceExtentionsAppNeed
										, VkPhysicalDevice physicalDevice) const;
	
	auto	checkIfPhysicalDeviceSuitable(VkPhysicalDevice device) const -> PhysicalDeviceData;
	auto	checkQueueFamilies(VkPhysicalDevice device) const -> QueueFamilies;
	auto	swapchainDetails(VkPhysicalDevice device) const -> SwapChainDetails;

	auto	chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& supportedFormats) -> VkSurfaceFormatKHR;
	auto	choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) -> VkPresentModeKHR;
	auto	chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities) -> VkExtent2D;

	auto	createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) -> VkImageView;

private:
	PhysicalDeviceData			m_physicalDeviceData = {};
	std::vector<SwapchainImage>	m_swapchainImages = {};
	Queues						m_queues = {};
	GLFWwindow*					m_window = nullptr;
	VkInstance					m_vkInstance = VK_NULL_HANDLE;
	VkPhysicalDevice			m_physicalDevice = VK_NULL_HANDLE;
	VkDevice					m_logicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR				m_surface = VK_NULL_HANDLE;
	VkSwapchainKHR				m_swapchain = VK_NULL_HANDLE;
	VkShaderModule				m_vertexShaderModule = VK_NULL_HANDLE;
	VkShaderModule				m_fragmentShaderModule = VK_NULL_HANDLE;
	VkRenderPass				m_renderPass = VK_NULL_HANDLE;
	VkPipelineLayout			m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline					m_graphicsPipeline = VK_NULL_HANDLE;

	VkSurfaceFormatKHR			m_surfaceFormat = {};
	VkExtent2D					m_imageExtent;
	VkPresentModeKHR			m_presentMode;

#ifdef NDEBUG
	const bool			enableValidationLayers = false;
#else
	const bool			enableValidationLayers = true;
#endif
};