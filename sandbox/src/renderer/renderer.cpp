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

constexpr std::array<const char*, Renderer::C_DEVICE_EXTEINTIONS_COUNT> deviceExtentions
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

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
	std::ifstream file(path, std::ios::binary);
	assert(file.is_open() && "Failed to open file");
	return { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
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
	createSurface();
	setupPhysicalDevice();
	createLogicalDevice();
	createSwapchain();
	createPipeline();
}

void Renderer::shutdown()
{
	for (auto& [_, imageView] : m_swapchainImages)
	{
		vkDestroyImageView(m_logicalDevice, imageView, nullptr);
	}

	if (m_swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(m_logicalDevice, m_swapchain, nullptr);
		m_swapchain = VK_NULL_HANDLE;
	}

	if (m_logicalDevice != VK_NULL_HANDLE)
	{
		vkDestroyDevice(m_logicalDevice, nullptr);
		m_logicalDevice = VK_NULL_HANDLE;
	}

	if (m_surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);
		m_surface = VK_NULL_HANDLE;
	}

	if (m_vkInstance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(m_vkInstance, nullptr);
		m_vkInstance = VK_NULL_HANDLE;
	}
}

void Renderer::update(float /*dt*/)
{

}

void Renderer::createVkInstance()
{
	//-- Instance creation info will need info about application
	//-- Most info here for developer
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Study";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	//-- This one is really important setting, this is vulkan apu version
	appInfo.apiVersion = VK_API_VERSION_1_4;

	//-- Creation info for a vkInstance
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
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
	VkResult res = vkCreateInstance(&createInfo, nullptr, &m_vkInstance);
	assert(res == VK_SUCCESS);
	assert(m_vkInstance != VK_NULL_HANDLE);
}

void Renderer::checkExtentionsSupport(const std::vector<const char*>& instanceExtentionsAppNeed) const
{
	uint32_t extentionsCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extentionsCount, nullptr);

	assert(extentionsCount >= instanceExtentionsAppNeed.size());
	std::vector<VkExtensionProperties> extentionsProps;
	//-- resize becuse we need to move current size to max extentions othervise standard functions are not gonna work properly
	extentionsProps.resize(extentionsCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extentionsCount, extentionsProps.data());

	for (const char* extention : instanceExtentionsAppNeed)
	{
		auto itRes = std::ranges::find_if(extentionsProps, [&](const VkExtensionProperties& vkInst)
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
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	assert(layerCount >= validationLayerAppNeed.size());
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layer : validationLayerAppNeed)
	{
		auto itRes = std::ranges::find_if(availableLayers, [&](const VkLayerProperties& vkInst)
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
	std::vector<VkDeviceQueueCreateInfo> queuesCreateInfos;
	std::set<int> queueFamilyIdices = {
		m_physicalDeviceData.m_queueFamilies.m_graphicQueue
		, m_physicalDeviceData.m_queueFamilies.m_presentationQueue
	};
	queuesCreateInfos.reserve(queueFamilyIdices.size());

	for (int queueIndex : queueFamilyIdices)
	{
		//-- Queue creation info
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueIndex;
		queueCreateInfo.queueCount = 1;
		float priority = 1.0f;
		queueCreateInfo.pQueuePriorities = &priority;

		queuesCreateInfos.push_back(queueCreateInfo);
	}

	//-- Device info itself
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = queuesCreateInfos.size();
	deviceCreateInfo.pQueueCreateInfos = queuesCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = deviceExtentions.size();
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtentions.data();

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	//-- Creating logical device
	VkResult result = vkCreateDevice(m_physicalDevice
		, &deviceCreateInfo
		, nullptr
		, &m_logicalDevice);
	assert(result == VK_SUCCESS);

	//-- Queues are created automatically, we save it
	vkGetDeviceQueue(m_logicalDevice
		, m_physicalDeviceData.m_queueFamilies.m_graphicQueue
		, 0
		, &m_queues.m_graphicQueue);

	vkGetDeviceQueue(m_logicalDevice
		, m_physicalDeviceData.m_queueFamilies.m_presentationQueue
		, 0
		, &m_queues.m_presentationQueue);
}

bool Renderer::checkDeviceExtentionsSupport(
	const std::array<const char*, C_DEVICE_EXTEINTIONS_COUNT>& deviceExtentionsAppNeed
	, VkPhysicalDevice physicalDevice) const
{
	uint32_t extentionsCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extentionsCount, nullptr);

	if (extentionsCount < deviceExtentionsAppNeed.size())
	{
		return false;
	}
	std::vector<VkExtensionProperties> extentionsProps;
	//-- resize becuse we need to move current size to max extentions othervise standard functions are not gonna work properly
	extentionsProps.resize(extentionsCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extentionsCount, extentionsProps.data());

	for (const char* extention : deviceExtentionsAppNeed)
	{
		auto itRes = std::ranges::find_if(extentionsProps, [&](const VkExtensionProperties& vkInst)
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
	VkResult result = glfwCreateWindowSurface(m_vkInstance, m_window, nullptr, &m_surface);
	assert(result == VK_SUCCESS);
}

void Renderer::createSwapchain()
{
	//-- VkSurfaceCapabilitiesKHR
	//-- VkSurfaceFormatKHR 
	//-- VkPresentModeKHR
	const SwapChainDetails&	swaphainDetails = m_physicalDeviceData.m_swapchainDetails;
	
	const VkSurfaceFormatKHR	surfaceFormat = chooseSurfaceFormat(swaphainDetails.m_surfaceSupportedFormats);
	const VkPresentModeKHR		presentMode = choosePresentMode(swaphainDetails.m_presentMode);
	const VkExtent2D			extent = chooseSwapChainExtent(swaphainDetails.m_surfaceCapabilities);

	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.imageFormat = surfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainCreateInfo.presentMode = presentMode;
	swapChainCreateInfo.imageExtent = extent;
	swapChainCreateInfo.minImageCount = std::clamp(swaphainDetails.m_surfaceCapabilities.minImageCount + 1
		, swaphainDetails.m_surfaceCapabilities.minImageCount
		, swaphainDetails.m_surfaceCapabilities.maxImageCount);
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.preTransform = swaphainDetails.m_surfaceCapabilities.currentTransform;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.clipped = VK_TRUE;

	const QueueFamilies& families = m_physicalDeviceData.m_queueFamilies;

	if (families.m_graphicQueue != families.m_presentationQueue)
	{
		const std::array<uint32_t, 2> indicies = {
			static_cast<uint32_t>(families.m_graphicQueue),
			static_cast<uint32_t>(families.m_presentationQueue)
		};
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapChainCreateInfo.queueFamilyIndexCount = 2;
		swapChainCreateInfo.pQueueFamilyIndices = indicies.data();
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.queueFamilyIndexCount = 0;
		swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	}
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
	swapChainCreateInfo.surface = m_surface;

	VkResult res = vkCreateSwapchainKHR(m_logicalDevice
		, &swapChainCreateInfo
		, nullptr
		, &m_swapchain);
	assert(res == VK_SUCCESS);

	uint32_t swapchainImagesCount = 0;
	vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &swapchainImagesCount, nullptr);
	std::vector<VkImage> images(swapchainImagesCount);
	vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &swapchainImagesCount, images.data());

	m_swapchainImages.reserve(swapchainImagesCount);
	for (auto& image : images)
	{
		m_swapchainImages.push_back({
				.m_image = image,
				.m_imageView = createImageView(image, surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT)
			});
	}

	m_surfaceFormat = surfaceFormat;
	m_presentMode = presentMode;
	m_imageExtent = extent;
}

void Renderer::createPipeline()
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
}

void Renderer::setupPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);
	assert(deviceCount > 0);
	std::vector<VkPhysicalDevice> physDevices;
	physDevices.resize(deviceCount);
	vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, physDevices.data());

	int bestScore = 0;
	for (VkPhysicalDevice& device : physDevices)
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
		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
		
		std::cout << std::format("Chosen physical device name name: {}"
			, properties.deviceName) << std::endl;
	}

	assert(bestScore > 0);
	assert(m_physicalDevice != VK_NULL_HANDLE);
	assert(m_physicalDeviceData.m_queueFamilies.isValid());
	//-- Maybe check if it supports specific modes we wanna see like mailbox & RGB8UNORM
	assert(!m_physicalDeviceData.m_swapchainDetails.m_presentMode.empty());
	assert(!m_physicalDeviceData.m_swapchainDetails.m_surfaceSupportedFormats.empty());
}

QueueFamilies Renderer::checkQueueFamilies(VkPhysicalDevice device) const
{
	QueueFamilies queueFamilies = {};

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	assert(queueFamilyCount > 0);

	std::vector<VkQueueFamilyProperties> queueFamilyProps;
	queueFamilyProps.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProps.data());

	int index = 0;
	for (auto& familyProp : queueFamilyProps)
	{
		if (familyProp.queueCount > 0)
		{
			//-- Check if queue has graphic family and queue count is not zero
			if (!!(familyProp.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				//-- Remember that queue index to work with that later
				queueFamilies.m_graphicQueue = index;
			}

			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device
				, index
				, m_surface
				, &presentSupport);
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

SwapChainDetails Renderer::swapchainDetails(VkPhysicalDevice device) const
{
	SwapChainDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.m_surfaceCapabilities);

	{
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.m_surfaceSupportedFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device
				, m_surface
				, &formatCount
				, details.m_surfaceSupportedFormats.data());
		}
	}

	{
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.m_presentMode.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.m_presentMode.data());
		}
	}

	return details;
}

VkSurfaceFormatKHR Renderer::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& supportedFormats)
{
	for (const auto& availableFormat : supportedFormats)
	{
		//-- BGR and nonlinear color space is the best for small indie game
		//-- since it's usually supported by the most monitors
		if ((availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB || availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB)
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}
	return supportedFormats[0];
}

VkPresentModeKHR Renderer::choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	//-- Looking for MAILBOX (best for games)
	//-- Tripple buffering, update on vertical blank and no tearing can be observed
	for (const auto& mode : availablePresentModes)
	{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return mode;
		}
	}
	//-- If there's no MAILBOX use FIFO (always available by specification)
	//-- "This is the only value of presentMode that is required to be supported."
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Renderer::chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	//-- If currExtent is max - than it will be handeled by surface
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	int width = 0;
	int height = 0;;
	glfwGetFramebufferSize(m_window, &width, &height);
	VkExtent2D ret = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
	ret.width = std::clamp(ret.width
		, capabilities.minImageExtent.width
		, capabilities.maxImageExtent.width);
	ret.height = std::clamp(ret.height
		, capabilities.minImageExtent.height
		, capabilities.maxImageExtent.height);

	return ret;
}

VkImageView Renderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;

	VkImageView res = {};
	assert(vkCreateImageView(m_logicalDevice, &createInfo, nullptr, &res) == VK_SUCCESS);
	return res;
}

PhysicalDeviceData Renderer::checkIfPhysicalDeviceSuitable(VkPhysicalDevice device) const
{
	PhysicalDeviceData data;

	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties(device, &properties);

	data.m_queueFamilies = checkQueueFamilies(device);
	if (data.m_queueFamilies.isValid())
	{
		data.m_score += 10;
	}

	//-- Device extentions
	if (checkDeviceExtentionsSupport(deviceExtentions, device))
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
	if (data.m_score > 0 && properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		data.m_score += 1;
	}

	//-- Information about what device can do (geom shaders, tess shaders, wide lines, etc)
	//VkPhysicalDeviceFeatures deviceFeatures = {};
	//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	return data;
}