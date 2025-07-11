#pragma once

#include <application/managers/events/event_interface.h>
//-- C version of interface only for capability with ImGui
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

struct EngineContext;

struct ImGuiInitInfo
{
	uint32_t			m_apiVersion;
	VkInstance			m_instance;
	VkPhysicalDevice	m_physicalDevice;
	VkDevice			m_device;
	uint32_t			m_queueFamily;
	VkQueue				m_queue;
	VkDescriptorPool	m_descriptorPool;
	VkRenderPass		m_renderPass;
	uint32_t			m_minImageCount;
	uint32_t			m_imageCount;
	GLFWwindow*			m_window;
};

class ImGuiIntegration
{
public:
	ImGuiIntegration() = default;
	ImGuiIntegration(ImGuiInitInfo& initInfo);

	void update(VkCommandBuffer command_buffer);

	void shutdown();
	
private:
	void setupStyle();
};