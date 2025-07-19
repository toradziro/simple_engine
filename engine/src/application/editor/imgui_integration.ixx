export module imgui_integration;

import <vector>;
import <functional>;

import <backends/imgui_impl_vulkan.h>;
import <backends/imgui_impl_glfw.h>;

import <vulkan/vulkan.h>;
import <GLFW/glfw3.h>;

import event_interface;

//-------------------------------------------------------------------------------------------------
export struct ImGuiInitInfo
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
export class ImGuiIntegration
{
public:
	//-------------------------------------------------------------------------------------------------
	ImGuiIntegration() = default;

	//-------------------------------------------------------------------------------------------------
	ImGuiIntegration(ImGuiInitInfo& initInfo)
	{
		ImGui_ImplVulkan_InitInfo imGuiVulkanInitInfo = {
			.ApiVersion = initInfo.m_apiVersion
			, .Instance = initInfo.m_instance
			, .PhysicalDevice = initInfo.m_physicalDevice
			, .Device = initInfo.m_device
			, .QueueFamily = initInfo.m_queueFamily
			, .Queue = initInfo.m_queue
			, .DescriptorPool = initInfo.m_descriptorPool
			, .RenderPass = initInfo.m_renderPass
			, .MinImageCount = initInfo.m_minImageCount
			, .ImageCount = initInfo.m_imageCount
		};

		ImGui::CreateContext();

		ImGui_ImplGlfw_InitForVulkan(initInfo.m_window, true);
		ImGui_ImplVulkan_Init(&imGuiVulkanInitInfo);

		setupStyle();
	}

	//-------------------------------------------------------------------------------------------------
	void update(VkCommandBuffer commandBuffer, std::vector<std::function<void()>>& drawListImGuiUI)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Invisible window
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		window_flags |= ImGuiWindowFlags_NoBackground;
		window_flags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		window_flags |= ImGuiWindowFlags_NoSavedSettings;

		// Remove visual elements
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		bool p_open = true;
		if (ImGui::Begin("##DockSpaceWindow", &p_open, window_flags))
		{
			ImGui::PopStyleVar(3);

			ImGuiID            dockspace_id = ImGui::GetID("MainDockSpace");
			ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags, nullptr);
		}
		else
		{
			ImGui::PopStyleVar(3);
		}
		ImGui::End();

		//ImGui::ShowDemoWindow();
		for (auto& drawItem : drawListImGuiUI)
		{
			drawItem();
		}

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

	//-------------------------------------------------------------------------------------------------
	void shutdown()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		ImGui::DestroyContext();
	}

private:
	//-------------------------------------------------------------------------------------------------
	void setupStyle()
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
};
