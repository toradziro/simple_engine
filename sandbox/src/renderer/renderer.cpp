#include "renderer.h"
#include <assert.h>
#include <ranges>

void Renderer::init(GLFWwindow* window)
{
	assert(window != nullptr);
	//m_window = window;

	createVkInstance();
	setupPhysicalDevice();
	createLogicalDevice();
}

void Renderer::shutdown()
{
	vkDestroyDevice(m_logicalDevice, nullptr);
	vkDestroyInstance(m_vkInstance, nullptr);
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
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = 0;

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

void Renderer::createLogicalDevice()
{
	//-- Queue for device info
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = m_physicalDeviceQueueFamilies.m_graphicQueue;
	queueCreateInfo.queueCount = 1;
	float priority = 1.0f;
	queueCreateInfo.pQueuePriorities = &priority;

	//-- Device info itself
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.ppEnabledExtensionNames = nullptr;

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
		, m_physicalDeviceQueueFamilies.m_graphicQueue
		, 0
		, &m_queues.m_graphicQueue);
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
		auto [currScore, queueFamilies] = checkIfPhysicalDeviceSuitable(device);

		if (currScore > bestScore)
		{
			bestScore = currScore;
			m_physicalDevice = device;
			m_physicalDeviceQueueFamilies = queueFamilies;
		}
	}
	assert(bestScore > 0);
	assert(m_physicalDevice != VK_NULL_HANDLE);
	assert(m_physicalDeviceQueueFamilies.isValid());
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
		//-- Check if queue has graphic family and queue count is not zero
		if (familyProp.queueCount > 0 && !!(familyProp.queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			//-- Remember that queue index to work with that later
			queueFamilies.m_graphicQueue = index;
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

std::pair<int, QueueFamilies> Renderer::checkIfPhysicalDeviceSuitable(VkPhysicalDevice device) const
{
	int score = 0;

	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties(device, &properties);

	QueueFamilies queueFamilies = checkQueueFamilies(device);
	if (queueFamilies.isValid())
	{
		score += 10;
	}

	//-- Prefer discrete videocard but only if it fits by queues reqs
	if (score > 0 && properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1;
	}

	//-- Information about what device can do (geom shaders, tess shaders, wide lines, etc)
	VkPhysicalDeviceFeatures deviceFeatures = {};
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	return { score, queueFamilies };
}