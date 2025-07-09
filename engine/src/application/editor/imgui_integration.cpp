#include "imgui_integration.h"

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include <application/engine.h>
#include <application/managers/window_manager.h>

ImGuiIntegration::ImGuiIntegration(ImGuiInitInfo& initInfo)
{
	ImGui_ImplVulkan_InitInfo imGuiVulkanInitInfo = {
		.ApiVersion		= initInfo.m_apiVersion,
		.Instance		= initInfo.m_instance,
		.PhysicalDevice	= initInfo.m_physicalDevice,
		.Device			= initInfo.m_device,
		.QueueFamily	= initInfo.m_queueFamily,
		.Queue			= initInfo.m_queue,
		.DescriptorPool	= initInfo.m_descriptorPool,
		.RenderPass		= initInfo.m_renderPass,
		.MinImageCount	= initInfo.m_minImageCount,
		.ImageCount		= initInfo.m_imageCount
	};

	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForVulkan(initInfo.m_window, true);
	ImGui_ImplVulkan_Init(&imGuiVulkanInitInfo);
}

void ImGuiIntegration::update(VkCommandBuffer commandBuffer)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();

	ImGui::Render();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void ImGuiIntegration::shutdown()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();

	ImGui::DestroyContext();
}
