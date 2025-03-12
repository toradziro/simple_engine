#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>

class Renderer
{
public:
	void init(GLFWwindow* window);
	void shutdown();
	void update(float /*dt*/);

private:
	void createVkInstance();
	void checkExtentionsSupport(const std::vector<const char*>& instanceExtentionsAppNeed);

	void setupPhysicalDevice();

private:
	//GLFWwindow*	m_window = nullptr;

	VkInstance			m_vkInstance = VK_NULL_HANDLE;
	VkPhysicalDevice	m_physicalDevice = VK_NULL_HANDLE;
	VkDevice			m_logicalDevice = VK_NULL_HANDLE;
};