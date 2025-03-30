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
	void	checkExtentionsSupport(const std::vector<const char*>& instanceExtentionsAppNeed) const;
	void	checkValidationLayerSupport(const std::vector<const char*>& validationLayerAppNeed) const;

	void	createLogicalDevice();
	void	checkDeviceExtentionsSupport(const std::array<const char*, C_DEVICE_EXTEINTIONS_COUNT>& deviceExtentionsAppNeed) const;
	void	createSurface();

	void							setupPhysicalDevice();
	std::pair<int, QueueFamilies>	checkIfPhysicalDeviceSuitable(VkPhysicalDevice device) const;
	QueueFamilies					checkQueueFamilies(VkPhysicalDevice device) const;

private:
	GLFWwindow*			m_window = nullptr;

	VkInstance			m_vkInstance = VK_NULL_HANDLE;
	VkPhysicalDevice	m_physicalDevice = VK_NULL_HANDLE;
	QueueFamilies		m_physicalDeviceQueueFamilies = {};
	Queues				m_queues = {};
	VkDevice			m_logicalDevice = VK_NULL_HANDLE;

	VkSurfaceKHR		m_surface = VK_NULL_HANDLE;

#ifdef NDEBUG
	const bool			enableValidationLayers = false;
#else
	const bool			enableValidationLayers = true;
#endif
};