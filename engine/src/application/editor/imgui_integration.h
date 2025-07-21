#pragma once

#include <vector>
#include <functional>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class Event;

//-------------------------------------------------------------------------------------------------
struct ImGuiInitInfo
{
	uint32_t         m_apiVersion;
	VkInstance       m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkDevice         m_device;
	uint32_t         m_queueFamily;
	VkQueue          m_queue;
	VkDescriptorPool m_descriptorPool;
	VkRenderPass     m_renderPass;
	uint32_t         m_minImageCount;
	uint32_t         m_imageCount;
	GLFWwindow*      m_window;
};

//-------------------------------------------------------------------------------------------------
class ImGuiIntegration
{
public:
	ImGuiIntegration() = default;
	ImGuiIntegration(ImGuiInitInfo& initInfo);

	void update(VkCommandBuffer commandBuffer, std::vector<std::function<void()>>& drawListImGuiUI);
	void shutdown();

private:
	void setupStyle();
};
