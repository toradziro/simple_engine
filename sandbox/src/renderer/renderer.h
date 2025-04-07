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

struct SwapChainDetails
{
	//-- Surface props such as size and buffer images count
	VkSurfaceCapabilitiesKHR		m_surfaceCapabilities = {};
	//-- Format such as RGBA8
	std::vector<VkSurfaceFormatKHR>	m_surfaceFormat;
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
	void	setupPhysicalDevice();

	void	checkExtentionsSupport(const std::vector<const char*>& instanceExtentionsAppNeed) const;
	void	checkValidationLayerSupport(const std::vector<const char*>& validationLayerAppNeed) const;
	bool	checkDeviceExtentionsSupport(const std::array<const char*, C_DEVICE_EXTEINTIONS_COUNT>& deviceExtentionsAppNeed
										, VkPhysicalDevice physicalDevice) const;
	
	PhysicalDeviceData	checkIfPhysicalDeviceSuitable(VkPhysicalDevice device) const;
	QueueFamilies		checkQueueFamilies(VkPhysicalDevice device) const;
	SwapChainDetails	swapchainDetails(VkPhysicalDevice device) const;

	VkSurfaceFormatKHR	chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& supportedFormats);
	VkPresentModeKHR	choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D			chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);

private:
	PhysicalDeviceData	m_physicalDeviceData = {};
	Queues				m_queues = {};
	GLFWwindow*			m_window = nullptr;
	VkInstance			m_vkInstance = VK_NULL_HANDLE;
	VkPhysicalDevice	m_physicalDevice = VK_NULL_HANDLE;
	VkDevice			m_logicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR		m_surface = VK_NULL_HANDLE;
	VkSwapchainKHR		m_swapchain = VK_NULL_HANDLE;

#ifdef NDEBUG
	const bool			enableValidationLayers = false;
#else
	const bool			enableValidationLayers = true;
#endif
};