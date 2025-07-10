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

	setupStyle();
}

void ImGuiIntegration::update(VkCommandBuffer commandBuffer)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//-- Test code
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

	ImGui::ShowDemoWindow();

	ImGui::Render();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		auto* context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(context);
	}
}

void ImGuiIntegration::shutdown()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();

	ImGui::DestroyContext();
}

void ImGuiIntegration::setupStyle()
{
	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	//-- WinBg
	style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	style.Colors[ImGuiCol_WindowBg].x = 0.11f;
	style.Colors[ImGuiCol_WindowBg].y = 0.11f;
	style.Colors[ImGuiCol_WindowBg].z = 0.11f;
	//-- FrameBg
	style.Colors[ImGuiCol_FrameBg].x = 0.25f;
	style.Colors[ImGuiCol_FrameBg].y = 0.25f;
	style.Colors[ImGuiCol_FrameBg].z = 0.25f;
	//-- FrameBgHovered
	style.Colors[ImGuiCol_FrameBgHovered].x = 0.5f;
	style.Colors[ImGuiCol_FrameBgHovered].y = 0.5f;
	style.Colors[ImGuiCol_FrameBgHovered].z = 0.5f;
	//-- FrameBgActive
	style.Colors[ImGuiCol_FrameBgActive].x = 0.8f;
	style.Colors[ImGuiCol_FrameBgActive].y = 0.8f;
	style.Colors[ImGuiCol_FrameBgActive].z = 0.8f;
	//-- TitleBg
	style.Colors[ImGuiCol_TitleBg].x = 0.15f;
	style.Colors[ImGuiCol_TitleBg].y = 0.15f;
	style.Colors[ImGuiCol_TitleBg].z = 0.15f;
	//-- TitleBgActive
	style.Colors[ImGuiCol_TitleBgActive].x = 0.3f;
	style.Colors[ImGuiCol_TitleBgActive].y = 0.3f;
	style.Colors[ImGuiCol_TitleBgActive].z = 0.3f;
	//-- Header
	style.Colors[ImGuiCol_Header].x = 0.4f;
	style.Colors[ImGuiCol_Header].y = 0.4f;
	style.Colors[ImGuiCol_Header].z = 0.4f;
	//-- Header Hovered
	style.Colors[ImGuiCol_HeaderHovered].x = 0.56f;
	style.Colors[ImGuiCol_HeaderHovered].y = 0.56f;
	style.Colors[ImGuiCol_HeaderHovered].z = 0.56f;
	//-- Tab
	style.Colors[ImGuiCol_Tab].x = 0.4f;
	style.Colors[ImGuiCol_Tab].y = 0.4f;
	style.Colors[ImGuiCol_Tab].z = 0.4f;
	//-- TabHovered
	style.Colors[ImGuiCol_TabHovered].x = 0.56f;
	style.Colors[ImGuiCol_TabHovered].y = 0.56f;
	style.Colors[ImGuiCol_TabHovered].z = 0.56f;
	//-- TabSelected
	style.Colors[ImGuiCol_TabSelected].x = 0.6f;
	style.Colors[ImGuiCol_TabSelected].y = 0.6f;
	style.Colors[ImGuiCol_TabSelected].z = 0.6f;

	style.Colors[ImGuiCol_TabSelectedOverline].x = 0.6f;
	style.Colors[ImGuiCol_TabSelectedOverline].y = 0.6f;
	style.Colors[ImGuiCol_TabSelectedOverline].z = 0.6f;

	style.Colors[ImGuiCol_TabDimmedSelected].x = 0.28f;
	style.Colors[ImGuiCol_TabDimmedSelected].y = 0.28f;
	style.Colors[ImGuiCol_TabDimmedSelected].z = 0.28f;

	style.Colors[ImGuiCol_Button].x = 0.555f;
	style.Colors[ImGuiCol_Button].y = 0.555f;
	style.Colors[ImGuiCol_Button].z = 0.555f;
}
