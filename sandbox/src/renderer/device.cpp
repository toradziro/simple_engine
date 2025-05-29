#include "device.h"

#include <cassert>
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
#include <cstdlib>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace
{

constexpr int	C_MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
#define VULKAN_CALL_CHECK(x) \
do { \
    vk::Result __res = (x); \
    if (__res != vk::Result::eSuccess) { \
        std::cerr << "Vulkan error: " << static_cast<int>(__res) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        throw std::runtime_error("Vulkan error"); \
    } \
} while(0)
#else
#define VULKAN_CALL_CHECK(x) \
do { \
    vk::Result __res = (x); \
    if (__res != vk::Result::eSuccess) { \
        std::cerr << "Vulkan error: " << static_cast<int>(__res) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        assert((x) == vk::Result::eSuccess); \
    } \
} while(0)
#endif

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

vk::VertexInputBindingDescription getBindingDescription()
{
	vk::VertexInputBindingDescription bindingDescription = {};
	bindingDescription.setBinding(0)
		.setStride(sizeof(VertexData))
		.setInputRate(vk::VertexInputRate::eVertex);
	return bindingDescription;
}

std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
{
	std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions = {};
	attributeDescriptions[0].setBinding(0)
		.setFormat(vk::Format::eR32G32Sfloat)
		.setLocation(0)
		.setOffset(offsetof(VertexData, m_vertex));

	attributeDescriptions[1].setBinding(0)
		.setFormat(vk::Format::eR32G32B32Sfloat)
		.setLocation(1)
		.setOffset(offsetof(VertexData, m_color));

	attributeDescriptions[2].setBinding(0)
		.setFormat(vk::Format::eR32G32Sfloat)
		.setLocation(2)
		.setOffset(offsetof(VertexData, m_texCoord));

	return attributeDescriptions;
}

}

VkGraphicDevice::~VkGraphicDevice()
{
	shutdown();
}

void VkGraphicDevice::init(GLFWwindow* window)
{
	assert(window != nullptr);
	m_window = window;

	try
	{
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
		std::cout << "createDescriptorSetLayout" << std::endl;
		createDescriptorSetLayout();
		std::cout << "createPipeline" << std::endl;
		createPipeline();
		std::cout << "createFramebuffer" << std::endl;
		createFramebuffer();
		std::cout << "createCommandPool" << std::endl;
		createCommandPool();
		std::cout << "createTextureImage" << std::endl;
		createTextureImage();
		std::cout << "createTextureImageView" << std::endl;
		createTextureImageView();
		std::cout << "createTextureSampler" << std::endl;
		createTextureSampler();
		std::cout << "createUniformBuffers" << std::endl;
		createUniformBuffers();
		std::cout << "createDescriptorPool" << std::endl;
		createDescriptorPool();
		std::cout << "createDescriptorSets" << std::endl;
		createDescriptorsSets();
		std::cout << "createCommandBuffer" << std::endl;
		createCommandBuffer();
		std::cout << "createSyncObjects" << std::endl;
		createSyncObjects();
		std::cout << "Vulkan objects initialized" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error during initialization: " << e.what() << std::endl;
		throw; // Re-throw to allow proper cleanup
	}
}

void VkGraphicDevice::shutdown()
{
	auto waitIdleGraphicQueueRes = m_queues.m_graphicQueue.waitIdle();
	auto waitIdlePresentationQueueRes = m_queues.m_presentationQueue.waitIdle();

	for (int i = 0; i < C_MAX_FRAMES_IN_FLIGHT; ++i)
	{
		m_logicalDevice.destroySemaphore(m_imageAvailableSemaphores[i]);
		m_logicalDevice.destroySemaphore(m_renderFinishedSemaphores[i]);
		m_logicalDevice.destroyFence(m_inFlightFences[i]);
	}
	for (const auto& buf : m_uniformBuffers)
	{
		m_logicalDevice.destroyBuffer(buf);
	}
	for (const auto& mem : m_uniformBuffersMemory)
	{
		m_logicalDevice.freeMemory(mem);
	}
	m_logicalDevice.destroyDescriptorPool(m_descriptorPool);
	m_logicalDevice.destroyDescriptorSetLayout(m_descriptorSetLayout);
	m_logicalDevice.freeCommandBuffers(m_commandPool, m_commandBuffers);
	m_logicalDevice.destroyCommandPool(m_commandPool);
	cleanupSwapchain();

	m_logicalDevice.destroySampler(m_textureSampler);
	m_logicalDevice.destroyImageView(m_textureImageView);
	m_logicalDevice.destroyImage(m_textureImage);
	m_logicalDevice.freeMemory(m_textureImageMemory);

	m_logicalDevice.destroyPipeline(m_graphicsPipeline);
	m_logicalDevice.destroyPipelineLayout(m_pipelineLayout);
	m_logicalDevice.destroyRenderPass(m_renderPass);
	m_logicalDevice.destroyShaderModule(m_vertexShaderModule);
	m_logicalDevice.destroyShaderModule(m_fragmentShaderModule);
	m_logicalDevice.destroy();

	m_vkInstance.destroySurfaceKHR(m_surface);
	m_vkInstance.destroy();
}

//void VkGraphicDevice::update(float dt)
//{
//	try
//	{
//		drawFrame(dt);
//	}
//	catch (const std::exception& e)
//	{
//		std::cerr << "Error during initialization: " << e.what() << std::endl;
//	}
//}

void VkGraphicDevice::beginFrame(float /*dt*/)
{
	VULKAN_CALL_CHECK(m_logicalDevice.waitForFences(m_inFlightFences[m_currFrame]
		, vk::True
		, UINT64_MAX));

	auto [result, imageIndex] = m_logicalDevice.acquireNextImageKHR(m_swapchain
		, UINT64_MAX
		, m_imageAvailableSemaphores[m_currFrame]);

	m_currImageIndex = imageIndex;

	if (result == vk::Result::eErrorOutOfDateKHR)
	{
		recreateSwapChain();
		return;
	}
	else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	//-- Only reset the fence if we are submitting work
	m_logicalDevice.resetFences(m_inFlightFences[m_currFrame]);

	m_commandBuffers[m_currFrame].reset();
}

void VkGraphicDevice::endFrame(const VulkanBufferMemory& vertices, const VulkanBufferMemory& indexBuffer, uint16_t spriteCount)
{
	recordCommandBuffer(m_commandBuffers[m_currFrame]
		, m_currImageIndex
		, vertices
		, indexBuffer
		, spriteCount);
	updateUniformBuffer();

	//-- Submitting command buffer
	vk::SubmitInfo submitInfo = {};
	vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	submitInfo.setWaitSemaphoreCount(1)
		.setWaitSemaphores(m_imageAvailableSemaphores[m_currFrame])
		.setWaitDstStageMask(waitStages)
		.setSignalSemaphoreCount(1)
		.setSignalSemaphores(m_renderFinishedSemaphores[m_currFrame])
		.setCommandBufferCount(1)
		.setCommandBuffers(m_commandBuffers[m_currFrame]);

	auto submitRes = m_queues.m_graphicQueue.submit(submitInfo, m_inFlightFences[m_currFrame]);

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.setWaitSemaphoreCount(1)
		.setWaitSemaphores(m_renderFinishedSemaphores[m_currFrame])
		.setSwapchainCount(1)
		.setSwapchains(m_swapchain)
		.setImageIndices(m_currImageIndex);

	auto resPresent = m_queues.m_presentationQueue.presentKHR(presentInfo);
	if (resPresent == vk::Result::eErrorOutOfDateKHR || m_framebufferResized)
	{
		m_framebufferResized = false;
		recreateSwapChain();
	}
	else if (resPresent != vk::Result::eSuccess && resPresent != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	m_currFrame = (m_currFrame + 1) % C_MAX_FRAMES_IN_FLIGHT;
}

void VkGraphicDevice::updateUniformBuffer()
{
	int w = 0, h = 0;
	glfwGetFramebufferSize(m_window, &w, &h);

	UniformBufferObject ubo = {};
	ubo.m_model = glm::mat4(1.0f);
	ubo.m_view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f)
		, glm::vec3(0.0f, 0.0f, 0.0f)
		, glm::vec3(0.0f, 1.0f, 0.0f));

	ubo.m_proj = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 10.0f);
	ubo.m_proj[1][1] *= -1;
	memcpy(m_uniformBuffersMapped[m_currFrame], &ubo, sizeof(UniformBufferObject));
}

uint32_t VkGraphicDevice::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties physDeviceMemProps = m_physicalDevice.getMemoryProperties();
	for (uint32_t i = 0; i < physDeviceMemProps.memoryTypeCount; ++i)
	{
		if ((typeFilter & (i << 1)) &&
			(physDeviceMemProps.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	assert(false);
	return 0;
}

void VkGraphicDevice::createVkInstance()
{
	//-- Instance creation info will need info about application
	//-- Most info here for developer
	vk::ApplicationInfo appInfo = {};
	appInfo.setPApplicationName("Vulkan Study")
		.setApiVersion(VK_API_VERSION_1_3);

	//-- Creation info for a vkInstance
	vk::InstanceCreateInfo createInfo = {};
	createInfo.setPApplicationInfo(&appInfo);

	//-- Create collection to hold instance extentions
	std::vector<const char*> instanceExtentions;

	uint32_t extentionsCount = 0;
	const char** glfwExtentionsList = glfwGetRequiredInstanceExtensions(&extentionsCount);
	instanceExtentions.reserve(extentionsCount);
	for (uint32_t i = 0; i < extentionsCount; ++i)
	{
		instanceExtentions.push_back(glfwExtentionsList[i]);
	}

	//-- Check if all requested extentions supported
	checkExtentionsSupport(instanceExtentions);

	//-- Set extentions we received from glfw
	createInfo.setEnabledExtensionCount(static_cast<uint32_t>(instanceExtentions.size()))
		.setPEnabledExtensionNames(instanceExtentions);

	//-- This variable we will use to enable validation layers
	std::vector<const char*> validationLayers;
	if (enableValidationLayers)
	{
		validationLayers.push_back("VK_LAYER_KHRONOS_validation");
		checkValidationLayerSupport(validationLayers);
		createInfo.setEnabledLayerCount(validationLayers.size());
	}
	else
	{
		createInfo.setEnabledLayerCount(0);
	}
	createInfo.setPEnabledLayerNames(validationLayers);

	//-- Create instance
	VULKAN_CALL_CHECK(vk::createInstance(&createInfo, nullptr, &m_vkInstance));
}

void VkGraphicDevice::checkExtentionsSupport(const std::vector<const char*>& instanceExtentionsAppNeed) const
{
	auto [res, extentionsProps] = vk::enumerateInstanceExtensionProperties();
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

void VkGraphicDevice::checkValidationLayerSupport(const std::vector<const char*>& validationLayerAppNeed) const
{
	auto [res, availableLayers] = vk::enumerateInstanceLayerProperties();

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

void VkGraphicDevice::createLogicalDevice()
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
	deviceFeatures.setSamplerAnisotropy(VK_TRUE);
	vk::DeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.setQueueCreateInfos(queuesCreateInfos)
		.setPEnabledFeatures(&deviceFeatures)
		.setEnabledExtensionCount(C_DEVICE_EXTENTIONS.size())
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

bool VkGraphicDevice::checkDeviceExtentionsSupport(const std::vector<const char*>& deviceExtentions, vk::PhysicalDevice physicalDevice) const
{
	auto [res, extentionsProps] = physicalDevice.enumerateDeviceExtensionProperties();

	for (const char* extention : deviceExtentions)
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

void VkGraphicDevice::createSurface()
{
	VkSurfaceKHR rawSurface = {};
	VkResult result = glfwCreateWindowSurface(m_vkInstance, m_window, nullptr, &rawSurface);
	assert(result == VK_SUCCESS);
	m_surface = vk::SurfaceKHR(rawSurface);
}

void VkGraphicDevice::recreateSwapChain()
{
	auto waitIdleRes = m_logicalDevice.waitIdle();

	cleanupSwapchain();

	m_swapchainImages.clear();
	m_swapChainFramebuffers.clear();
	m_physicalDeviceData.m_swapchainDetails = swapchainDetails(m_physicalDevice);

	createSwapchain();
	createFramebuffer();
}

void VkGraphicDevice::createSwapchain()
{
	const SwapChainDetails& swaphainDetails = m_physicalDeviceData.m_swapchainDetails;

	const vk::SurfaceFormatKHR	surfaceFormat = chooseSurfaceFormat(swaphainDetails.m_surfaceSupportedFormats);
	const vk::PresentModeKHR	presentMode = choosePresentMode(swaphainDetails.m_presentMode);
	const vk::Extent2D			extent = chooseSwapChainExtent(swaphainDetails.m_surfaceCapabilities);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.setImageFormat(surfaceFormat.format)
		.setImageColorSpace(surfaceFormat.colorSpace)
		.setPresentMode(presentMode)
		.setImageExtent(extent)
		.setMinImageCount(std::clamp(swaphainDetails.m_surfaceCapabilities.minImageCount + 1
			, swaphainDetails.m_surfaceCapabilities.minImageCount
			, swaphainDetails.m_surfaceCapabilities.maxImageCount))
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setPreTransform(swaphainDetails.m_surfaceCapabilities.currentTransform)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setClipped(VK_TRUE);

	const QueueFamilies& families = m_physicalDeviceData.m_queueFamilies;

	if (families.m_graphicQueue != families.m_presentationQueue)
	{
		const std::array<uint32_t, 2> indicies = {
			static_cast<uint32_t>(families.m_graphicQueue),
			static_cast<uint32_t>(families.m_presentationQueue)
		};
		swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
			.setQueueFamilyIndexCount(2)
			.setPQueueFamilyIndices(indicies.data());
	}
	else
	{
		swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive)
			.setQueueFamilyIndexCount(0)
			.setPQueueFamilyIndices(nullptr);
	}
	swapChainCreateInfo.setOldSwapchain(VK_NULL_HANDLE)
		.setSurface(m_surface);

	VULKAN_CALL_CHECK(m_logicalDevice.createSwapchainKHR(&swapChainCreateInfo
		, nullptr
		, &m_swapchain));

	auto [res, images] = m_logicalDevice.getSwapchainImagesKHR(m_swapchain);
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

void VkGraphicDevice::createShaderModule()
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

	std::cout << "Successfully compiled shaders" << std::endl;

	vk::ShaderModuleCreateInfo vertexShaderModuleCreateInfo = {};
	vertexShaderModuleCreateInfo.setCodeSize(compiled_vertex_shader.size() * sizeof(uint32_t))
		.setPCode(compiled_vertex_shader.data());
	VULKAN_CALL_CHECK(m_logicalDevice.createShaderModule(&vertexShaderModuleCreateInfo
		, nullptr
		, &m_vertexShaderModule));

	vk::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo = {};
	fragmentShaderModuleCreateInfo.setCodeSize(compiled_fragment_shader.size() * sizeof(uint32_t))
		.setPCode(compiled_fragment_shader.data());
	VULKAN_CALL_CHECK(m_logicalDevice.createShaderModule(&fragmentShaderModuleCreateInfo
		, nullptr
		, &m_fragmentShaderModule));
}

void VkGraphicDevice::createDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setBinding(0)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	vk::DescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1)
		.setBinding(1)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	std::array<vk::DescriptorSetLayoutBinding, 2> layoutBindings = {
		layoutBinding
		, samplerLayoutBinding
	};

	vk::DescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.setBindingCount(1)
		.setBindings(layoutBindings);

	auto [res, layout] = m_logicalDevice.createDescriptorSetLayout(createInfo);
	m_descriptorSetLayout = layout;
}

void VkGraphicDevice::createPipeline()
{
	vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
	vertexShaderStageCreateInfo.setStage(vk::ShaderStageFlagBits::eVertex)
		.setModule(m_vertexShaderModule)
		.setPName("main");

	vk::PipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
	fragmentShaderStageCreateInfo.setStage(vk::ShaderStageFlagBits::eFragment)
		.setModule(m_fragmentShaderModule)
		.setPName("main");

	std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

	//-- Create pipline now
	//-- Vertex input
	auto bindingDescriptions = getBindingDescription();
	auto atributeDescriptions = getAttributeDescriptions();
	vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.setVertexBindingDescriptions(bindingDescriptions)
		.setVertexAttributeDescriptions(atributeDescriptions);

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.setTopology(vk::PrimitiveTopology::eTriangleList)
		.setPrimitiveRestartEnable(false);

	vk::Viewport viewport = {};
	viewport.setX(0.0f)
		.setY(0.0f)
		.setWidth(m_imageExtent.width)
		.setHeight(m_imageExtent.height)
		.setMinDepth(0.0f)
		.setMaxDepth(1.0f);

	vk::Rect2D scissor = {};
	scissor.setOffset(vk::Offset2D(0, 0))
		.setExtent(m_imageExtent);

	vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.setViewportCount(1)
		.setPViewports(&viewport)
		.setScissorCount(1)
		.setPScissors(&scissor);

	//-- DYNAMIC STATE
	const std::array<vk::DynamicState, 2> dynamicStateEnables = {
		vk::DynamicState::eViewport, //-- allow to resize in command buffer with vkCmdSetViewport
		vk::DynamicState::eScissor
	};

	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.setDynamicStateCount(dynamicStateEnables.size())
		.setPDynamicStates(dynamicStateEnables.data());

	//-- RASTERIZER
	vk::PipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.setDepthClampEnable(VK_FALSE)
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(vk::PolygonMode::eFill) //-- Try line
		.setLineWidth(1.0f)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setDepthBiasEnable(VK_FALSE);

	vk::PipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.setSampleShadingEnable(VK_FALSE)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR
		| vk::ColorComponentFlagBits::eG
		| vk::ColorComponentFlagBits::eB
		| vk::ColorComponentFlagBits::eA)
		.setBlendEnable(VK_TRUE)
		.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
		.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
		.setColorBlendOp(vk::BlendOp::eAdd)
		.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
		.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
		.setAlphaBlendOp(vk::BlendOp::eAdd);

	//-- TODO: Check how to use it
	vk::PipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.setLogicOpEnable(VK_FALSE)
		.setLogicOp(vk::LogicOp::eCopy) // Optional
		.setAttachmentCount(1)
		.setPAttachments(&colorBlendAttachment);

	//-- We will need layout for uniforms
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.setSetLayouts(m_descriptorSetLayout)
		.setSetLayoutCount(1);

	VULKAN_CALL_CHECK(m_logicalDevice.createPipelineLayout(&pipelineLayoutInfo
		, nullptr
		, &m_pipelineLayout));

	vk::GraphicsPipelineCreateInfo pipelineInfo = {};
	//-- Static part of pypline
	pipelineInfo.setStageCount(2)
		.setPStages(shaderStages.data())
		.setPVertexInputState(&vertexInputCreateInfo)
		.setPInputAssemblyState(&inputAssemblyCreateInfo)
		.setPViewportState(&viewportStateCreateInfo)
		.setPRasterizationState(&rasterizerCreateInfo)
		.setPMultisampleState(&multisampling)
		.setPDepthStencilState(nullptr) // Optional
		.setPColorBlendState(&colorBlending)
		.setPDynamicState(&dynamicStateCreateInfo)
		//-- Layout, describing uniforms
		.setLayout(m_pipelineLayout)
		//-- Render pass
		.setRenderPass(m_renderPass)
		.setSubpass(0);

	VULKAN_CALL_CHECK(m_logicalDevice.createGraphicsPipelines(VK_NULL_HANDLE
		, 1
		, &pipelineInfo
		, nullptr
		, &m_graphicsPipeline));
}

void VkGraphicDevice::createRenderPass()
{
	vk::AttachmentDescription colorAttachment = {};
	colorAttachment.setFormat(m_surfaceFormat.format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::SubpassDescription subpass = {};
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorAttachmentRef);

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

void VkGraphicDevice::createFramebuffer()
{
	m_swapChainFramebuffers.resize(m_swapchainImages.size());
	int i = 0;
	for (const auto& [_, imageView] : m_swapchainImages)
	{
		std::array<vk::ImageView, 1> attachments = { imageView };

		vk::FramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.setRenderPass(m_renderPass)
			.setAttachmentCount(1)
			.setPAttachments(attachments.data())
			.setWidth(m_imageExtent.width)
			.setHeight(m_imageExtent.height)
			.setLayers(1);

		VULKAN_CALL_CHECK(m_logicalDevice.createFramebuffer(&framebufferInfo
			, nullptr
			, &m_swapChainFramebuffers[i]));

		++i;
	}
}

void VkGraphicDevice::createCommandPool()
{
	vk::CommandPoolCreateInfo poolInfo = {};
	poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		.setQueueFamilyIndex(m_physicalDeviceData.m_queueFamilies.m_graphicQueue);

	VULKAN_CALL_CHECK(m_logicalDevice.createCommandPool(&poolInfo
		, nullptr
		, &m_commandPool));
}

void VkGraphicDevice::createTextureImage()
{
	constexpr auto C_IMAGE_PATH = "images/nyan_cat.png";

	auto curr_path = std::filesystem::current_path();
	auto root_path = curr_path.parent_path();

	auto full_cat_path = root_path / C_IMAGE_PATH;

	int texture_width = 0;
	int texture_height = 0;
	int tex_channels = 0;

	stbi_set_flip_vertically_on_load(true);
	stbi_uc* pixels = stbi_load(full_cat_path.string().c_str()
		, &texture_width
		, &texture_height
		, &tex_channels
		, STBI_rgb_alpha);

	if (!pixels)
	{
		assert(false);
	}

	//-- 4 bytes per pixel
	vk::DeviceSize imageSize = texture_width * texture_height * 4;

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;

	createBuffer(imageSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		stagingBuffer,
		stagingBufferMemory);

	void* data = nullptr;
	auto res = m_logicalDevice.mapMemory(stagingBufferMemory, 0, imageSize, {}, &data);
	memcpy(data, pixels, imageSize);

	m_logicalDevice.unmapMemory(stagingBufferMemory);
	stbi_image_free(pixels);

	createImage(texture_width
		, texture_height
		, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled
		, vk::MemoryPropertyFlagBits::eDeviceLocal
		, m_textureImage
		, m_textureImageMemory);

	transitionImage(m_textureImage,
		vk::Format::eR8G8B8A8Srgb,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal);

	copyBufferToImage(stagingBuffer,
		m_textureImage,
		texture_width,
		texture_height);

	transitionImage(m_textureImage,
		vk::Format::eR8G8B8A8Srgb,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal);

	m_logicalDevice.destroyBuffer(stagingBuffer);
	m_logicalDevice.freeMemory(stagingBufferMemory);
}

void VkGraphicDevice::createTextureImageView()
{
	m_textureImageView = createImageView(m_textureImage
		, vk::Format::eR8G8B8A8Srgb
		, vk::ImageAspectFlagBits::eColor);
}

void VkGraphicDevice::createTextureSampler()
{
	const auto props = m_physicalDevice.getProperties();
	const auto maxAnisotropy = props.limits.maxSamplerAnisotropy;
	vk::SamplerCreateInfo createInfo = {};
	createInfo.setMagFilter(vk::Filter::eNearest)
		.setMinFilter(vk::Filter::eNearest)
		.setAddressModeU(vk::SamplerAddressMode::eRepeat)
		.setAddressModeV(vk::SamplerAddressMode::eRepeat)
		.setAddressModeW(vk::SamplerAddressMode::eRepeat)
		.setAnisotropyEnable(VK_TRUE)
		.setMaxAnisotropy(maxAnisotropy)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMipmapMode(vk::SamplerMipmapMode::eNearest)
		.setMipLodBias(0.0f)
		.setMinLod(0.0f)
		.setMaxLod(0.0f);

	auto [res, sampler] = m_logicalDevice.createSampler(createInfo);
	m_textureSampler = sampler;
}

VulkanBufferMemory VkGraphicDevice::createCombinedVertexBuffer(
	const std::vector<std::array<VertexData, 4>>& sprites)
{
	size_t totalVertices = sprites.size() * 4;
	auto bufferSize = totalVertices * sizeof(VertexData);

	VulkanBufferMemory resultMemory;
	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;

	createBuffer(bufferSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		stagingBuffer,
		stagingBufferMemory);

	void* data = nullptr;
	auto mapResult = m_logicalDevice.mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);

	VertexData* vertexData = static_cast<VertexData*>(data);
	size_t vertexIndex = 0;

	memcpy(data, sprites.data(), bufferSize);

	m_logicalDevice.unmapMemory(stagingBufferMemory);

	createBuffer(bufferSize,
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		resultMemory.m_Buffer,
		resultMemory.m_BufferMem);

	copyBuffer(stagingBuffer, resultMemory.m_Buffer, bufferSize);

	m_logicalDevice.destroyBuffer(stagingBuffer);
	m_logicalDevice.freeMemory(stagingBufferMemory);

	return resultMemory;
}

uint8_t VkGraphicDevice::maxFrames() const
{
	return C_MAX_FRAMES_IN_FLIGHT;
}

uint8_t VkGraphicDevice::currFrame() const
{
	return m_currFrame;
}

void VkGraphicDevice::waitGraphicIdle()
{
	m_queues.m_graphicQueue.waitIdle();
}

auto VkGraphicDevice::createIndexBuffer(uint16_t spriteCount) -> VulkanBufferMemory
{
	std::vector<uint16_t> indicies;
	indicies.resize(spriteCount * 6);
	constexpr uint32_t C_VERTICES_IN_QUAD = 4;
	uint32_t offset = 0;

	for (uint32_t sprite = 0; sprite < spriteCount; ++sprite)
	{
		uint32_t baseIndex = sprite * 6;

		indicies[baseIndex + 0] = offset + 0;
		indicies[baseIndex + 1] = offset + 1;
		indicies[baseIndex + 2] = offset + 2;

		indicies[baseIndex + 3] = offset + 2;
		indicies[baseIndex + 4] = offset + 3;
		indicies[baseIndex + 5] = offset + 0;

		offset += C_VERTICES_IN_QUAD;
	}

	VulkanBufferMemory resultMemory;

	auto bufferSize = indicies.size() * sizeof(uint16_t);

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;

	createBuffer(bufferSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		stagingBuffer,
		stagingBufferMemory);

	void* data = nullptr;
	auto res = m_logicalDevice.mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);
	memcpy(data, indicies.data(), bufferSize);

	m_logicalDevice.unmapMemory(stagingBufferMemory);

	createBuffer(bufferSize,
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		resultMemory.m_Buffer,
		resultMemory.m_BufferMem);

	copyBuffer(stagingBuffer, resultMemory.m_Buffer, bufferSize);
	m_logicalDevice.destroyBuffer(stagingBuffer);
	m_logicalDevice.freeMemory(stagingBufferMemory);

	return resultMemory;
}

void VkGraphicDevice::createUniformBuffers()
{
	auto bufferSize = sizeof(UniformBufferObject);

	m_uniformBuffers.resize(C_MAX_FRAMES_IN_FLIGHT);
	m_uniformBuffersMemory.resize(C_MAX_FRAMES_IN_FLIGHT);
	m_uniformBuffersMapped.resize(C_MAX_FRAMES_IN_FLIGHT);

	for (uint32_t i = 0; i < C_MAX_FRAMES_IN_FLIGHT; ++i)
	{
		createBuffer(bufferSize,
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			m_uniformBuffers[i],
			m_uniformBuffersMemory[i]);

		auto res = m_logicalDevice.mapMemory(m_uniformBuffersMemory[i],
			0,
			bufferSize,
			{},
			&m_uniformBuffersMapped[i]);
	}
}

void VkGraphicDevice::clearBuffer(VulkanBufferMemory memory)
{
	m_logicalDevice.destroyBuffer(memory.m_Buffer);
	m_logicalDevice.freeMemory(memory.m_BufferMem);
}

void VkGraphicDevice::createDescriptorPool()
{
	std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].setDescriptorCount(C_MAX_FRAMES_IN_FLIGHT)
		.setType(vk::DescriptorType::eUniformBuffer);
	poolSizes[1].setDescriptorCount(C_MAX_FRAMES_IN_FLIGHT)
		.setType(vk::DescriptorType::eCombinedImageSampler);

	vk::DescriptorPoolCreateInfo createInfo = {};
	createInfo.setPoolSizeCount(1)
		.setPoolSizes(poolSizes)
		.setMaxSets(C_MAX_FRAMES_IN_FLIGHT);

	auto [res, pool] = m_logicalDevice.createDescriptorPool(createInfo);
	m_descriptorPool = pool;
}

void VkGraphicDevice::createDescriptorsSets()
{
	std::vector<vk::DescriptorSetLayout> layouts(C_MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo = {};
	allocInfo.setDescriptorPool(m_descriptorPool)
		.setSetLayouts(layouts);

	auto res = m_logicalDevice.allocateDescriptorSets(allocInfo);
	m_descriptorSets = res.value;
	for (uint32_t i = 0; i < C_MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vk::DescriptorBufferInfo bufferInfo = {};
		bufferInfo.setBuffer(m_uniformBuffers[i])
			.setOffset(0)
			.setRange(sizeof(UniformBufferObject));

		vk::DescriptorImageInfo imageInfo = {};
		imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(m_textureImageView)
			.setSampler(m_textureSampler);

		std::array<vk::WriteDescriptorSet, 2> descriptorsWrite = {};
		descriptorsWrite[0].setBufferInfo(bufferInfo)
			.setDstSet(m_descriptorSets[i])
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1);
		descriptorsWrite[1].setImageInfo(imageInfo)
			.setDstSet(m_descriptorSets[i])
			.setDstBinding(1)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1);

		m_logicalDevice.updateDescriptorSets(descriptorsWrite, {});
	}
}

void VkGraphicDevice::createCommandBuffer()
{
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.setCommandBufferCount(C_MAX_FRAMES_IN_FLIGHT)
		.setCommandPool(m_commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary);

	auto [res, commandBuffers] = m_logicalDevice.allocateCommandBuffers(commandBufferAllocateInfo);
	m_commandBuffers = commandBuffers;
}

void VkGraphicDevice::createSyncObjects()
{
	m_imageAvailableSemaphores.reserve(C_MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.reserve(C_MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.reserve(C_MAX_FRAMES_IN_FLIGHT);

	vk::FenceCreateInfo fenceInfo = {};
	fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

	for (int i = 0; i < C_MAX_FRAMES_IN_FLIGHT; ++i)
	{
		{
			auto [res, imageAvailableSemaphore] = m_logicalDevice.createSemaphore(vk::SemaphoreCreateInfo());
			m_imageAvailableSemaphores.push_back(std::move(imageAvailableSemaphore));
		}

		{
			auto [res, renderFinishedSemaphore] = m_logicalDevice.createSemaphore(vk::SemaphoreCreateInfo());
			m_renderFinishedSemaphores.push_back(std::move(renderFinishedSemaphore));
		}

		{
			auto [res, fence] = m_logicalDevice.createFence(fenceInfo);
			m_inFlightFences.push_back(std::move(fence));
		}
	}

}

void VkGraphicDevice::setupPhysicalDevice()
{
	auto [res, physDevices] = m_vkInstance.enumeratePhysicalDevices();

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

void VkGraphicDevice::recordCommandBuffer(vk::CommandBuffer commandBuffer
	, uint32_t imageIndex
	, const VulkanBufferMemory& vertices
	, const VulkanBufferMemory& indexBuffer
	, uint16_t spriteCount)
{
	vk::CommandBufferBeginInfo cmdBBeginfo = {};
	VULKAN_CALL_CHECK(commandBuffer.begin(&cmdBBeginfo));

	vk::RenderPassBeginInfo renderPassInfo = {};
	vk::Rect2D renderArea = {};
	vk::ClearValue clearValue = { vk::ClearColorValue(0.2f, 0.2f, 0.2f, 1.0f) };
	renderArea.setOffset({ 0 , 0 }).setExtent(m_imageExtent);
	renderPassInfo.setRenderPass(m_renderPass)
		.setFramebuffer(m_swapChainFramebuffers[imageIndex])
		.setRenderArea(renderArea)
		.setClearValueCount(1)
		.setClearValues({ clearValue });

	commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline);

	commandBuffer.bindVertexBuffers(0, vertices.m_Buffer, { 0 });
	commandBuffer.bindIndexBuffer(indexBuffer.m_Buffer, 0, vk::IndexType::eUint16);

	vk::Viewport viewport = {};
	viewport.setX(0.0f).setY(0.0f)
		.setWidth(m_imageExtent.width)
		.setHeight(m_imageExtent.height)
		.setMinDepth(0.0f)
		.setMaxDepth(1.0f);
	commandBuffer.setViewport(0, viewport);

	vk::Rect2D scissor = {};
	scissor.setExtent(m_imageExtent).setOffset({ 0, 0 });
	commandBuffer.setScissor(0, scissor);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		m_pipelineLayout,
		0,
		m_descriptorSets[m_currFrame], {});

	commandBuffer.drawIndexed(spriteCount * 6, 1, 0, 0, 0);
	commandBuffer.endRenderPass();
	auto res = commandBuffer.end();
}

QueueFamilies VkGraphicDevice::checkQueueFamilies(vk::PhysicalDevice device) const
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

SwapChainDetails VkGraphicDevice::swapchainDetails(vk::PhysicalDevice device) const
{
	SwapChainDetails details;
	VULKAN_CALL_CHECK(device.getSurfaceCapabilitiesKHR(m_surface, &details.m_surfaceCapabilities));

	{
		auto [res, surfaceSupportedFormats] = device.getSurfaceFormatsKHR(m_surface);
		details.m_surfaceSupportedFormats = std::move(surfaceSupportedFormats);
	}

	{
		auto [res, surfacePresentModes] = device.getSurfacePresentModesKHR(m_surface);
		details.m_presentMode = std::move(surfacePresentModes);
	}

	return details;
}

vk::SurfaceFormatKHR VkGraphicDevice::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& supportedFormats)
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

vk::PresentModeKHR VkGraphicDevice::choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
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

vk::Extent2D VkGraphicDevice::chooseSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
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

vk::ImageView VkGraphicDevice::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags)
{
	vk::ImageViewCreateInfo createInfo = {};
	createInfo.setImage(image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(format);

	createInfo.subresourceRange.setAspectMask(aspectFlags)
		.setBaseMipLevel(0)
		.setLayerCount(1)
		.setLevelCount(1)
		.setBaseArrayLayer(0);

	vk::ImageView res = {};
	VULKAN_CALL_CHECK(m_logicalDevice.createImageView(&createInfo, nullptr, &res));
	return res;
}

void VkGraphicDevice::createBuffer(vk::DeviceSize size
	, vk::BufferUsageFlags usageFlags
	, vk::MemoryPropertyFlags memPropFlags
	, vk::Buffer& buffer
	, vk::DeviceMemory& deviceMemory)
{
	vk::BufferCreateInfo bufferInfo = {};
	bufferInfo.setSize(size)
		.setUsage(usageFlags)
		.setSharingMode(vk::SharingMode::eExclusive);

	{
		auto [res, createdBuffer] = m_logicalDevice.createBuffer(bufferInfo);
		buffer = createdBuffer;
	}

	vk::MemoryRequirements memReq = m_logicalDevice.getBufferMemoryRequirements(buffer);
	uint32_t memType = findMemoryType(memReq.memoryTypeBits, memPropFlags);

	vk::MemoryAllocateInfo allocInfo = {};
	allocInfo.setMemoryTypeIndex(memType)
		.setAllocationSize(memReq.size);

	{
		auto [res, allocatedMemory] = m_logicalDevice.allocateMemory(allocInfo);
		VULKAN_CALL_CHECK(res);
		deviceMemory = allocatedMemory;
		auto bindBufferRes = m_logicalDevice.bindBufferMemory(buffer, deviceMemory, 0);
	}
}

void VkGraphicDevice::createImage(uint32_t width
	, uint32_t height
	, vk::ImageUsageFlags usage
	, vk::MemoryPropertyFlags memPropFlags
	, vk::Image& image
	, vk::DeviceMemory& imageMemory)
{
	vk::ImageCreateInfo createInfo = {};
	createInfo.setImageType(vk::ImageType::e2D)
		.setExtent({ width, height, 1 })
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(vk::Format::eR8G8B8A8Srgb)
		.setTiling(vk::ImageTiling::eOptimal)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setUsage(usage)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setSamples(vk::SampleCountFlagBits::e1);

	auto [result, createdImage] = m_logicalDevice.createImage(createInfo);
	image = createdImage;

	vk::MemoryRequirements memReq = {};
	memReq = m_logicalDevice.getImageMemoryRequirements(image);

	uint32_t memType = findMemoryType(memReq.memoryTypeBits, memPropFlags);

	vk::MemoryAllocateInfo allocInfo = {};
	allocInfo.setMemoryTypeIndex(memType)
		.setAllocationSize(memReq.size);

	{
		auto [res, allocatedMemory] = m_logicalDevice.allocateMemory(allocInfo);
		VULKAN_CALL_CHECK(res);
		imageMemory = allocatedMemory;
		auto bindImageRes = m_logicalDevice.bindImageMemory(image, imageMemory, 0);
		VULKAN_CALL_CHECK(bindImageRes);
	}
}

void VkGraphicDevice::transitionImage(vk::Image image
	, vk::Format format
	, vk::ImageLayout oldLayout
	, vk::ImageLayout newLayout)
{
	auto commandBuffer = beginSingleTimeCommands();

	vk::AccessFlags srcAccess = {};
	vk::AccessFlags dstAccess = {};
	vk::PipelineStageFlags srcStage = {};
	vk::PipelineStageFlags dstStage = {};

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		srcAccess = vk::AccessFlagBits::eNone;
		dstAccess = vk::AccessFlagBits::eTransferWrite;
		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else
	{
		srcAccess = vk::AccessFlagBits::eTransferWrite;
		dstAccess = vk::AccessFlagBits::eShaderRead;
		srcStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader;
	}

	vk::ImageMemoryBarrier barrier = {};
	barrier.setOldLayout(oldLayout)
		.setNewLayout(newLayout)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setImage(image)
		.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 })
		.setSrcAccessMask(srcAccess)
		.setDstAccessMask(dstAccess);

	commandBuffer.pipelineBarrier(srcStage
		, dstStage
		, vk::DependencyFlagBits::eByRegion
		, {}
		, {}
	, barrier);

	endSingleTimeCommand(commandBuffer);
}

void VkGraphicDevice::copyBufferToImage(vk::Buffer buffer
	, vk::Image image
	, uint32_t width
	, uint32_t height)
{
	auto commandBuffer = beginSingleTimeCommands();

	vk::BufferImageCopy region = {};
	region.setBufferOffset(0)
		.setBufferRowLength(0)
		.setBufferImageHeight(0)
		.setImageSubresource({ vk::ImageAspectFlagBits::eColor, 0, 0, 1 })
		.setImageOffset({ 0, 0, 0 })
		.setImageExtent({ width, height, 1 });

	commandBuffer.copyBufferToImage(buffer
		, image
		, vk::ImageLayout::eTransferDstOptimal
		, { region });

	endSingleTimeCommand(commandBuffer);
}

void VkGraphicDevice::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
	auto commandBuffer = beginSingleTimeCommands();

	vk::BufferCopy bufferCopy = {};
	bufferCopy.setSize(size)
		.setSrcOffset(0)
		.setDstOffset(0);

	commandBuffer.copyBuffer(srcBuffer, dstBuffer, { bufferCopy });

	endSingleTimeCommand(commandBuffer);
}

vk::CommandBuffer VkGraphicDevice::beginSingleTimeCommands()
{
	vk::CommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandPool(m_commandPool)
		.setCommandBufferCount(1);

	vk::CommandBuffer commandBuffer = {};
	auto [allocRes, commandBuffers] = m_logicalDevice.allocateCommandBuffers(allocateInfo);
	commandBuffer = *commandBuffers.begin();

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	auto cmdBeginRes = commandBuffer.begin(beginInfo);
	return commandBuffer;
}

void VkGraphicDevice::endSingleTimeCommand(vk::CommandBuffer commandBuffer)
{
	auto res = commandBuffer.end();

	vk::SubmitInfo submitInfo = {};
	submitInfo.setCommandBufferCount(1)
		.setCommandBuffers({ commandBuffer });

	res = m_queues.m_graphicQueue.submit(submitInfo);
	res = m_queues.m_graphicQueue.waitIdle();

	m_logicalDevice.freeCommandBuffers(m_commandPool, { commandBuffer });
}

void VkGraphicDevice::cleanupSwapchain()
{
	for (auto& framebuffer : m_swapChainFramebuffers)
	{
		m_logicalDevice.destroyFramebuffer(framebuffer);
	}
	for (auto& [_, imageView] : m_swapchainImages)
	{
		m_logicalDevice.destroyImageView(imageView);
	}
	m_logicalDevice.destroySwapchainKHR(m_swapchain);
}

PhysicalDeviceData VkGraphicDevice::checkIfPhysicalDeviceSuitable(vk::PhysicalDevice device) const
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