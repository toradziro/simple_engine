#include "imgui_system.h"

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include <application/engine.h>
#include <application/managers/window_manager.h>

/*
struct ImGui_ImplVulkan_InitInfo
{
	uint32_t                        ApiVersion;                 // Fill with API version of Instance, e.g. VK_API_VERSION_1_3 or your value of VkApplicationInfo::apiVersion. May be lower than header version (VK_HEADER_VERSION_COMPLETE)
	VkInstance                      Instance;
	VkPhysicalDevice                PhysicalDevice;
	VkDevice                        Device;
	uint32_t                        QueueFamily;
	VkQueue                         Queue;
	VkDescriptorPool                DescriptorPool;             // See requirements in note above; ignored if using DescriptorPoolSize > 0
	VkRenderPass                    RenderPass;                 // Ignored if using dynamic rendering
	uint32_t                        MinImageCount;              // >= 2
	uint32_t                        ImageCount;                 // >= MinImageCount
	VkSampleCountFlagBits           MSAASamples;                // 0 defaults to VK_SAMPLE_COUNT_1_BIT

	// (Optional)
	VkPipelineCache                 PipelineCache;
	uint32_t                        Subpass;

	// (Optional) Set to create internal descriptor pool instead of using DescriptorPool
	uint32_t                        DescriptorPoolSize;

	// (Optional) Dynamic Rendering
	// Need to explicitly enable VK_KHR_dynamic_rendering extension to use this, even for Vulkan 1.3.
	bool                            UseDynamicRendering;
#ifdef IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
	VkPipelineRenderingCreateInfoKHR PipelineRenderingCreateInfo;
#endif

	// (Optional) Allocation, Debugging
	const VkAllocationCallbacks*    Allocator;
	void                            (*CheckVkResultFn)(VkResult err);
	VkDeviceSize                    MinAllocationSize;          // Minimum allocation size. Set to 1024*1024 to satisfy zealous best practices validation layer and waste a little memory.
};
*/

ImGuiSystem::ImGuiSystem(EngineContext& context) : m_engineContext(context)
{
	auto& winManager = m_engineContext.m_managerHolder.getManager<WindowManager>();

	ImGui_ImplGlfw_InitForVulkan(winManager.window(), true);
	ImGui_ImplVulkan_InitInfo imGuiVulkanInitInfo;
	ImGui_ImplVulkan_Init(&imGuiVulkanInitInfo);
}

ImGuiSystem::~ImGuiSystem()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiSystem::update(float dt)
{
	ImGui::Begin("Test Window");
	ImGui::End();
}

void ImGuiSystem::onEvent(Event& event)
{
}
