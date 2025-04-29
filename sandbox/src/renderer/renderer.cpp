#include "renderer.h"

#include <assert.h>
#include <ranges>
#include <set>
#include <string>
#include <iostream>
#include <format>
#include <algorithm>
#include <filesystem>
#include <cstdio>
#include <fstream>
#include <shaderc/shaderc.hpp>

namespace
{

#define VULKAN_CALL_CHECK(x) assert((x) == vk::Result::eSuccess)

std::vector<uint32_t> compileShaderFromSource(const std::string& source,
	shaderc_shader_kind kind,
	const std::string& name)
{
	shaderc::Compiler		compiler;
	shaderc::CompileOptions	options;

	options.SetOptimizationLevel(shaderc_optimization_level_performance);

	auto result = compiler.CompileGlslToSpv(source, kind, name.c_str(), options);

	assert(result.GetCompilationStatus() == shaderc_compilation_status_success);

	// Spirv binary code
	return { result.cbegin(), result.cend() };
}

std::string readFile(const std::string& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	assert(file && file.is_open());

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::string buffer;
	buffer.resize(size);

	if (!file.read(buffer.data(), size))
	{
		assert(false);
	}

	return buffer;
}

}

Renderer::~Renderer()
{
	shutdown();
}

void Renderer::init(GLFWwindow* window)
{
	assert(window != nullptr);
	m_window = window;

	createVkInstance();
	//-- Create surface earlier than other devices types since we need it
	//-- in checking queue that can support presentation operations
	std::cout << "createSurface" << std::endl;
	createSurface();
	std::cout << "setupPhysicalDevice" << std::endl;
	setupPhysicalDevice();
	std::cout << "createLogicalDevice" << std::endl;
	createLogicalDevice();
	std::cout << "createSwapchain" << std::endl;
	createSwapchain();
	std::cout << "createShaderModule" << std::endl;
	createShaderModule();
	std::cout << "createRenderPass" << std::endl;
	createRenderPass();
	std::cout << "createPipeline" << std::endl;
	createPipeline();
	std::cout << "createFramebuffer" << std::endl;
	createFramebuffer();
	std::cout << "createCommandPool" << std::endl;
	createCommandPool();
	std::cout << "createCommandBuffer" << std::endl;
	createCommandBuffer();
	std::cout << "createSyncObjects" << std::endl;
	createSyncObjects();
	std::cout << "Vulkan objects initialized" << std::endl;
}

void Renderer::shutdown()
{
	m_queues.m_graphicQueue.waitIdle();
	m_queues.m_presentationQueue.waitIdle();

	m_logicalDevice.destroySemaphore(m_imageAvailableSemaphore);
	m_logicalDevice.destroySemaphore(m_renderFinishedSemaphore);
	m_logicalDevice.destroyFence(m_inFlightFence);
	m_logicalDevice.destroyCommandPool(m_commandPool);
	for (auto& framebuffer : m_swapChainFramebuffers)
	{
		m_logicalDevice.destroyFramebuffer(framebuffer);
	}
	m_logicalDevice.destroyPipeline(m_graphicsPipeline);
	m_logicalDevice.destroyPipelineLayout(m_pipelineLayout);
	m_logicalDevice.destroyRenderPass(m_renderPass);
	m_logicalDevice.destroyShaderModule(m_vertexShaderModule);
	m_logicalDevice.destroyShaderModule(m_fragmentShaderModule);
	for (auto& [_, imageView] : m_swapchainImages)
	{
		m_logicalDevice.destroyImageView(imageView);
	}
	m_logicalDevice.destroySwapchainKHR(m_swapchain);
	m_logicalDevice.destroy();
	
	m_vkInstance.destroySurfaceKHR(m_surface);
	m_vkInstance.destroy();
}

void Renderer::update(float dt)
{
	drawFrame(dt);
}

void Renderer::drawFrame(float /*dt*/)
{
	VULKAN_CALL_CHECK(m_logicalDevice.waitForFences(m_inFlightFence, vk::True, UINT64_MAX));
	m_logicalDevice.resetFences(m_inFlightFence);
	auto [result, imageIndex] = m_logicalDevice.acquireNextImageKHR(m_swapchain, UINT64_MAX, m_imageAvailableSemaphore);
	VULKAN_CALL_CHECK(result);
	m_commandBuffer.reset();
	recordCommandBuffer(m_commandBuffer, imageIndex);

	//-- Submitting command buffer
	vk::SubmitInfo submitInfo = {};
	vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	submitInfo.setWaitSemaphoreCount(1)
		.setWaitSemaphores(m_imageAvailableSemaphore)
		.setWaitDstStageMask(waitStages)
		.setSignalSemaphoreCount(1)
		.setSignalSemaphores(m_renderFinishedSemaphore)
		.setCommandBufferCount(1)
		.setCommandBuffers(m_commandBuffer);

	m_queues.m_graphicQueue.submit(submitInfo, m_inFlightFence);

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.setWaitSemaphoreCount(1)
		.setWaitSemaphores(m_renderFinishedSemaphore)
		.setSwapchainCount(1)
		.setSwapchains(m_swapchain)
		.setImageIndices(imageIndex);

	[[maybe_unused]]auto res = m_queues.m_presentationQueue.presentKHR(presentInfo);
}

void Renderer::createVkInstance()
{
	//-- Instance creation info will need info about application
	//-- Most info here for developer
	vk::ApplicationInfo appInfo = {};
	appInfo.pApplicationName = "Vulkan Study";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	//-- This one is really important setting, this is vulkan apu version
	appInfo.apiVersion = VK_API_VERSION_1_4;

	//-- Creation info for a vkInstance
	vk::InstanceCreateInfo createInfo = {};
	createInfo.pApplicationInfo = &appInfo;

	//-- Create collection to hold instance extentions
	std::vector<const char*> instanceExtentions;

	uint32_t extentionsCount = 0;
	const char** glfwExtentionsList = glfwGetRequiredInstanceExtensions(&extentionsCount);
	assert(extentionsCount > 0);
	instanceExtentions.reserve(extentionsCount);
	for (uint32_t i = 0; i < extentionsCount; ++i)
	{
		instanceExtentions.push_back(glfwExtentionsList[i]);
	}

	//-- Check if all requested extentions supported
	checkExtentionsSupport(instanceExtentions);

	//-- Set extentions we received from glfw
	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtentions.size());
	createInfo.ppEnabledExtensionNames = instanceExtentions.data();

	//-- This variable we will use to enable validation layers

	std::vector<const char*> validationLayers;
	if (enableValidationLayers)
	{
		validationLayers.push_back("VK_LAYER_KHRONOS_validation");
		checkValidationLayerSupport(validationLayers);
		createInfo.enabledLayerCount = validationLayers.size();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}
	createInfo.ppEnabledLayerNames = validationLayers.data();

	//-- Create instance
	VULKAN_CALL_CHECK(vk::createInstance(&createInfo, nullptr, &m_vkInstance));
}

void Renderer::checkExtentionsSupport(const std::vector<const char*>& instanceExtentionsAppNeed) const
{
	std::vector<vk::ExtensionProperties> extentionsProps = vk::enumerateInstanceExtensionProperties();
	for (const char* extention : instanceExtentionsAppNeed)
	{
		auto itRes = std::ranges::find_if(extentionsProps, [&](const vk::ExtensionProperties& vkInst)
			{
				if (strcmp(vkInst.extensionName, extention) == 0)
				{
					return true;
				}
				return false;
			});
		assert(itRes != extentionsProps.end());
	}
}

void Renderer::checkValidationLayerSupport(const std::vector<const char*>& validationLayerAppNeed) const
{
	std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

	for (const char* layer : validationLayerAppNeed)
	{
		auto itRes = std::ranges::find_if(availableLayers, [&](const vk::LayerProperties& vkInst)
			{
				if (strcmp(vkInst.layerName, layer) == 0)
				{
					return true;
				}
				return false;
			});
		assert(itRes != availableLayers.end());
	}
}

void Renderer::createLogicalDevice()
{
	//-- Vector for queue creation information and set to avoid duplication same queue
	std::vector<vk::DeviceQueueCreateInfo> queuesCreateInfos;
	std::set<int> queueFamilyIdices = {
		m_physicalDeviceData.m_queueFamilies.m_graphicQueue
		, m_physicalDeviceData.m_queueFamilies.m_presentationQueue
	};
	queuesCreateInfos.reserve(queueFamilyIdices.size());

	for (int queueIndex : queueFamilyIdices)
	{
		//-- Queue creation info
		const float priority = 1.0f;
		vk::DeviceQueueCreateInfo queueCreateInfo({}, queueIndex, 1, &priority);
		queuesCreateInfos.push_back(queueCreateInfo);
	}

	//-- Device info itself
	vk::PhysicalDeviceFeatures deviceFeatures = {};
	vk::DeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.setQueueCreateInfos(queuesCreateInfos)
		.setPEnabledFeatures(&deviceFeatures)
		.setPEnabledExtensionNames(C_DEVICE_EXTENTIONS);

	//-- Creating logical device
	VULKAN_CALL_CHECK(m_physicalDevice.createDevice(&deviceCreateInfo
		, nullptr
		, &m_logicalDevice));

	//-- Queues are created automatically, we save it
	m_logicalDevice.getQueue(m_physicalDeviceData.m_queueFamilies.m_graphicQueue
		, 0
		, &m_queues.m_graphicQueue);

	m_logicalDevice.getQueue(m_physicalDeviceData.m_queueFamilies.m_presentationQueue
		, 0
		, &m_queues.m_presentationQueue);
}

bool Renderer::checkDeviceExtentionsSupport(const std::vector<const char*>& deviceExts, vk::PhysicalDevice physicalDevice) const
{
	std::vector<vk::ExtensionProperties> extentionsProps = physicalDevice.enumerateDeviceExtensionProperties();

	for (const char* extention : deviceExts)
	{
		auto itRes = std::ranges::find_if(extentionsProps, [&](const vk::ExtensionProperties& vkInst)
			{
				if (strcmp(vkInst.extensionName, extention) == 0)
				{
					return true;
				}
				return false;
			});
		if (itRes == extentionsProps.end())
		{
			return false;
		}
	}
	return true;
}

void Renderer::createSurface()
{
	VkSurfaceKHR rawSurface = {};
	VkResult result = glfwCreateWindowSurface(m_vkInstance, m_window, nullptr, &rawSurface);
	assert(result == VK_SUCCESS);
	m_surface = vk::SurfaceKHR(rawSurface);
}

void Renderer::createSwapchain()
{
	const SwapChainDetails&	swaphainDetails = m_physicalDeviceData.m_swapchainDetails;
	
	const vk::SurfaceFormatKHR	surfaceFormat = chooseSurfaceFormat(swaphainDetails.m_surfaceSupportedFormats);
	const vk::PresentModeKHR	presentMode = choosePresentMode(swaphainDetails.m_presentMode);
	const vk::Extent2D			extent = chooseSwapChainExtent(swaphainDetails.m_surfaceCapabilities);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.imageFormat = surfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainCreateInfo.presentMode = presentMode;
	swapChainCreateInfo.imageExtent = extent;
	swapChainCreateInfo.minImageCount = std::clamp(swaphainDetails.m_surfaceCapabilities.minImageCount + 1
		, swaphainDetails.m_surfaceCapabilities.minImageCount
		, swaphainDetails.m_surfaceCapabilities.maxImageCount);
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	swapChainCreateInfo.preTransform = swaphainDetails.m_surfaceCapabilities.currentTransform;
	swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapChainCreateInfo.clipped = VK_TRUE;

	const QueueFamilies& families = m_physicalDeviceData.m_queueFamilies;

	if (families.m_graphicQueue != families.m_presentationQueue)
	{
		const std::array<uint32_t, 2> indicies = {
			static_cast<uint32_t>(families.m_graphicQueue),
			static_cast<uint32_t>(families.m_presentationQueue)
		};
		swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		swapChainCreateInfo.queueFamilyIndexCount = 2;
		swapChainCreateInfo.pQueueFamilyIndices = indicies.data();
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
		swapChainCreateInfo.queueFamilyIndexCount = 0;
		swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	}
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
	swapChainCreateInfo.surface = m_surface;

	VULKAN_CALL_CHECK(m_logicalDevice.createSwapchainKHR(&swapChainCreateInfo
		, nullptr
		, &m_swapchain));

	std::vector<vk::Image> images = m_logicalDevice.getSwapchainImagesKHR(m_swapchain);
	for (auto& image : images)
	{
		m_swapchainImages.push_back({
				.m_image = image,
				.m_imageView = createImageView(image, surfaceFormat.format, vk::ImageAspectFlagBits::eColor)
			});
	}

	m_surfaceFormat = surfaceFormat;
	m_presentMode = presentMode;
	m_imageExtent = extent;
}

void Renderer::createShaderModule()
{
	constexpr auto C_V_SHADER = "shaders/hello.vert";
	constexpr auto C_F_SHADER = "shaders/hello.frag";

	auto curr_path = std::filesystem::current_path();
	auto root_path = curr_path.parent_path();

	auto full_vertex_shader_path = root_path / C_V_SHADER;
	auto full_fragment_shader_path = root_path / C_F_SHADER;

	std::cout << std::format("Vertex Shader path: '{}'", full_vertex_shader_path.generic_string()) << std::endl;
	std::cout << std::format("Fragment Shader path: '{}'", full_fragment_shader_path.generic_string()) << std::endl;

	std::vector<uint32_t> compiled_vertex_shader = compileShaderFromSource(
		readFile(full_vertex_shader_path.generic_string()), shaderc_vertex_shader, "test_vertex_shader"
	);
	std::vector<uint32_t> compiled_fragment_shader = compileShaderFromSource(
		readFile(full_fragment_shader_path.generic_string()), shaderc_fragment_shader, "test_fragment_shader"
	);

	std::cout << "Sucessfully compiled shaders" << std::endl;

	vk::ShaderModuleCreateInfo vertexShaderModuleCreateInfo = {};
	vertexShaderModuleCreateInfo.codeSize = compiled_vertex_shader.size() * sizeof(uint32_t);
	vertexShaderModuleCreateInfo.pCode = compiled_vertex_shader.data();
	VULKAN_CALL_CHECK(m_logicalDevice.createShaderModule(&vertexShaderModuleCreateInfo
		, nullptr
		, &m_vertexShaderModule));

	vk::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo = {};
	fragmentShaderModuleCreateInfo.codeSize = compiled_fragment_shader.size() * sizeof(uint32_t);
	fragmentShaderModuleCreateInfo.pCode = compiled_fragment_shader.data();
	VULKAN_CALL_CHECK(m_logicalDevice.createShaderModule(&fragmentShaderModuleCreateInfo
		, nullptr
		, &m_fragmentShaderModule));
}

void Renderer::createPipeline()
{
	vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
	vertexShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eVertex;
	vertexShaderStageCreateInfo.module = m_vertexShaderModule;
	vertexShaderStageCreateInfo.pName = "main";

	vk::PipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
	fragmentShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fragmentShaderStageCreateInfo.module = m_fragmentShaderModule;
	fragmentShaderStageCreateInfo.pName = "main";

	std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

	//-- Create pipline now
	//-- Vertex input
	vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
	vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssemblyCreateInfo.primitiveRestartEnable = false;

	vk::Viewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = m_imageExtent.width;
	viewport.height = m_imageExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor = {};
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = m_imageExtent;

	vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	//-- DYNAMIC STATE
	const std::array<vk::DynamicState, 2> dynamicStateEnables = {
		vk::DynamicState::eViewport, //-- allow to resize in command buffer with vkCmdSetViewport
		vk::DynamicState::eScissor
	};

	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.dynamicStateCount = dynamicStateEnables.size();
	dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();

	//-- RASTERIZER
	vk::PipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.polygonMode = vk::PolygonMode::eFill;
	rasterizerCreateInfo.lineWidth = 1.0f;
	rasterizerCreateInfo.cullMode = vk::CullModeFlagBits::eBack;
	rasterizerCreateInfo.frontFace = vk::FrontFace::eClockwise;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;

	vk::PipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

	vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR
		| vk::ColorComponentFlagBits::eG
		| vk::ColorComponentFlagBits::eB
		| vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = VK_FALSE;

	//-- TODO: Check how to use it
	vk::PipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = vk::LogicOp::eCopy; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	//-- We will need layout for uniforms
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.setLayoutCount = 0;				// Optional
	pipelineLayoutInfo.pSetLayouts = nullptr;			// Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0;		// Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr;	// Optional

	VULKAN_CALL_CHECK(m_logicalDevice.createPipelineLayout(&pipelineLayoutInfo
		, nullptr
		, &m_pipelineLayout));

	vk::GraphicsPipelineCreateInfo pipelineInfo = {};
	//-- Static part of pypline
	pipelineInfo.stageCount				= 2;
	pipelineInfo.pStages				= shaderStages.data();
	pipelineInfo.pVertexInputState		= &vertexInputCreateInfo;
	pipelineInfo.pInputAssemblyState	= &inputAssemblyCreateInfo;
	pipelineInfo.pViewportState			= &viewportStateCreateInfo;
	pipelineInfo.pRasterizationState	= &rasterizerCreateInfo;
	pipelineInfo.pMultisampleState		= &multisampling;
	pipelineInfo.pDepthStencilState		= nullptr; // Optional
	pipelineInfo.pColorBlendState		= &colorBlending;
	pipelineInfo.pDynamicState			= &dynamicStateCreateInfo;

	//-- Layout, describing uniforms
	pipelineInfo.layout = m_pipelineLayout;

	//-- Render pass
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.subpass = 0;
	VULKAN_CALL_CHECK(m_logicalDevice.createGraphicsPipelines(VK_NULL_HANDLE
		, 1
		, &pipelineInfo
		, nullptr
		, &m_graphicsPipeline));
}

void Renderer::createRenderPass()
{
	vk::AttachmentDescription colorAttachment = {};
	colorAttachment.format = m_surfaceFormat.format;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	vk::SubpassDependency dependency = {};
	dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

	vk::RenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.setAttachmentCount(1)
		.setAttachments(colorAttachment)
		.setSubpassCount(1)
		.setSubpasses(subpass)
		.setDependencyCount(1)
		.setDependencies(dependency);

	VULKAN_CALL_CHECK(m_logicalDevice.createRenderPass(&renderPassInfo
			, nullptr
			, &m_renderPass));

}

void Renderer::createFramebuffer()
{
	m_swapChainFramebuffers.resize(m_swapchainImages.size());
	int i = 0;
	for (const auto& [_, imageView] : m_swapchainImages)
	{
		std::array<vk::ImageView, 1> attachments = { imageView };

		vk::FramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.renderPass		= m_renderPass;
		framebufferInfo.attachmentCount	= 1;
		framebufferInfo.pAttachments	= attachments.data();
		framebufferInfo.width			= m_imageExtent.width;
		framebufferInfo.height			= m_imageExtent.height;
		framebufferInfo.layers			= 1;

		VULKAN_CALL_CHECK(m_logicalDevice.createFramebuffer(&framebufferInfo
			, nullptr
			, &m_swapChainFramebuffers[i]));

		++i;
	}
}

void Renderer::createCommandPool()
{
	vk::CommandPoolCreateInfo poolInfo = {};
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	poolInfo.queueFamilyIndex = m_physicalDeviceData.m_queueFamilies.m_graphicQueue;

	VULKAN_CALL_CHECK(m_logicalDevice.createCommandPool(&poolInfo
		, nullptr
		, &m_commandPool));
}

void Renderer::createCommandBuffer()
{
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.setCommandBufferCount(1)
		.setCommandPool(m_commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary);

	auto allocatedCommandBuffers = m_logicalDevice.allocateCommandBuffers(commandBufferAllocateInfo);
	m_commandBuffer = *allocatedCommandBuffers.begin();
}

void Renderer::createSyncObjects()
{
	m_imageAvailableSemaphore = m_logicalDevice.createSemaphore(vk::SemaphoreCreateInfo());
	m_renderFinishedSemaphore = m_logicalDevice.createSemaphore(vk::SemaphoreCreateInfo());
	vk::FenceCreateInfo fenceInfo = {};
	fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
	m_inFlightFence = m_logicalDevice.createFence(fenceInfo);
}

void Renderer::setupPhysicalDevice()
{
	std::vector<vk::PhysicalDevice> physDevices = m_vkInstance.enumeratePhysicalDevices();

	int bestScore = 0;
	for (vk::PhysicalDevice& device : physDevices)
	{
		//-- Looking for the best device
		PhysicalDeviceData physDeviceData = checkIfPhysicalDeviceSuitable(device);

		if (physDeviceData.m_score > bestScore)
		{
			bestScore = physDeviceData.m_score;
			m_physicalDevice = device;
			m_physicalDeviceData = physDeviceData;
		}
	}

	{
		vk::PhysicalDeviceProperties properties = m_physicalDevice.getProperties();
		std::cout << std::format("Chosen physical device name name: {}"
			, properties.deviceName.operator std::string()) << std::endl;
	}

	assert(bestScore > 0);
	assert(m_physicalDevice != VK_NULL_HANDLE);
	assert(m_physicalDeviceData.m_queueFamilies.isValid());
	//-- Maybe check if it supports specific modes we wanna see like mailbox & RGB8UNORM
	assert(!m_physicalDeviceData.m_swapchainDetails.m_presentMode.empty());
	assert(!m_physicalDeviceData.m_swapchainDetails.m_surfaceSupportedFormats.empty());
}

void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	vk::CommandBufferBeginInfo cmdBBeginfo = {};
	VULKAN_CALL_CHECK(m_commandBuffer.begin(&cmdBBeginfo));

	vk::RenderPassBeginInfo renderPassInfo = {};
	vk::Rect2D renderArea = {};
	vk::ClearValue clearValue = { vk::ClearColorValue(0.0f, 0.0f, 1.0f, 1.0f) };
	renderArea.setOffset({ 0 , 0 }).setExtent(m_imageExtent);
	renderPassInfo.setRenderPass(m_renderPass)
		.setFramebuffer(m_swapChainFramebuffers[imageIndex])
		.setRenderArea(renderArea)
		.setClearValueCount(1)
		.setClearValues({ clearValue });

	m_commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline);
	vk::Viewport viewport = {};
	viewport.setX(0.0f).setY(0.0f)
		.setWidth(m_imageExtent.width)
		.setHeight(m_imageExtent.height)
		.setMinDepth(0.0f)
		.setMaxDepth(0.0f);
	m_commandBuffer.setViewport(0, viewport);

	vk::Rect2D scissor = {};
	scissor.setExtent(m_imageExtent).setOffset({ 0, 0 });
	m_commandBuffer.setScissor(0, scissor);
	m_commandBuffer.draw(3, 1, 0, 0);
	m_commandBuffer.endRenderPass();
	m_commandBuffer.end();
}

QueueFamilies Renderer::checkQueueFamilies(vk::PhysicalDevice device) const
{
	QueueFamilies queueFamilies = {};
	std::vector<vk::QueueFamilyProperties> queueFamilyProps = device.getQueueFamilyProperties();

	int index = 0;
	for (auto& familyProp : queueFamilyProps)
	{
		if (familyProp.queueCount > 0)
		{
			//-- Check if queue has graphic family and queue count is not zero
			if (!!(familyProp.queueFlags & vk::QueueFlagBits::eGraphics))
			{
				//-- Remember that queue index to work with that later
				queueFamilies.m_graphicQueue = index;
			}

			vk::Bool32 presentSupport = VK_FALSE;
			VULKAN_CALL_CHECK(device.getSurfaceSupportKHR(index
				, m_surface
				, &presentSupport));
			if (presentSupport == VK_TRUE)
			{
				queueFamilies.m_presentationQueue = index;
			}
		}
		if (queueFamilies.isValid())
		{
			//-- Already found needed queues
			break;
		}

		++index;
	}

	return queueFamilies;
}

SwapChainDetails Renderer::swapchainDetails(vk::PhysicalDevice device) const
{
	SwapChainDetails details;
	VULKAN_CALL_CHECK(device.getSurfaceCapabilitiesKHR(m_surface, &details.m_surfaceCapabilities));

	details.m_surfaceSupportedFormats = device.getSurfaceFormatsKHR(m_surface);
	details.m_presentMode = device.getSurfacePresentModesKHR(m_surface);

	return details;
}

vk::SurfaceFormatKHR Renderer::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& supportedFormats)
{
	for (const auto& availableFormat : supportedFormats)
	{
		//-- BGR and nonlinear color space is the best for small indie game
		//-- since it's usually supported by the most monitors
		if ((availableFormat.format == vk::Format::eB8G8R8A8Srgb || availableFormat.format == vk::Format::eR8G8B8A8Srgb)
			&& availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return availableFormat;
		}
	}
	return supportedFormats[0];
}

vk::PresentModeKHR Renderer::choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
	//-- Looking for MAILBOX (best for games)
	//-- Tripple buffering, update on vertical blank and no tearing can be observed
	for (const auto& mode : availablePresentModes)
	{
		if (mode == vk::PresentModeKHR::eMailbox)
		{
			return mode;
		}
	}
	//-- If there's no MAILBOX use FIFO (always available by specification)
	//-- "This is the only value of presentMode that is required to be supported."
	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Renderer::chooseSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
	//-- If currExtent is max - than it will be handeled by surface
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	int width = 0;
	int height = 0;;
	glfwGetFramebufferSize(m_window, &width, &height);
	vk::Extent2D ret = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
	ret.width = std::clamp(ret.width
		, capabilities.minImageExtent.width
		, capabilities.maxImageExtent.width);
	ret.height = std::clamp(ret.height
		, capabilities.minImageExtent.height
		, capabilities.maxImageExtent.height);

	return ret;
}

vk::ImageView Renderer::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags)
{
	vk::ImageViewCreateInfo createInfo = {};
	createInfo.image = image;
	createInfo.viewType = vk::ImageViewType::e2D;
	createInfo.format = format;

	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;

	vk::ImageView res = {};
	VULKAN_CALL_CHECK(m_logicalDevice.createImageView(&createInfo, nullptr, &res));
	return res;
}

PhysicalDeviceData Renderer::checkIfPhysicalDeviceSuitable(vk::PhysicalDevice device) const
{
	PhysicalDeviceData data;

	vk::PhysicalDeviceProperties properties = device.getProperties();

	data.m_queueFamilies = checkQueueFamilies(device);
	if (data.m_queueFamilies.isValid())
	{
		data.m_score += 10;
	}

	//-- Device extentions
	if (checkDeviceExtentionsSupport(C_DEVICE_EXTENTIONS, device))
	{
		data.m_score += 10;
	}

	data.m_swapchainDetails = swapchainDetails(device);
	bool swapchainValid = !data.m_swapchainDetails.m_presentMode.empty()
		&& !data.m_swapchainDetails.m_surfaceSupportedFormats.empty();
	if (swapchainValid)
	{
		data.m_score += 10;
	}

	//-- Prefer discrete videocard but only if it fits by queues reqs
	if (data.m_score > 0 && properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
	{
		data.m_score += 1;
	}

	//-- Information about what device can do (geom shaders, tess shaders, wide lines, etc)
	//vk::PhysicalDeviceFeatures deviceFeatures = {};
	//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	return data;
}