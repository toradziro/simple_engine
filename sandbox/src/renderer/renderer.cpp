#include "renderer.h"
#include <assert.h>
#include <ranges>

void Renderer::init(GLFWwindow* window)
{
	assert(window != nullptr);
	//m_window = window;

	createVkInstance();
	setupPhysicalDevice();
}

void Renderer::shutdown()
{
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

void Renderer::checkExtentionsSupport(const std::vector<const char*>& instanceExtentionsAppNeed)
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

void Renderer::setupPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);
	assert(deviceCount > 0);
	std::vector<VkPhysicalDevice> physDevices;
	physDevices.resize(deviceCount);
	vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, physDevices.data());

	int bestScore = -1;
	for (VkPhysicalDevice& device : physDevices)
	{
		int currScore = 0;

		//-- Prefer discrete videocard
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			currScore += 1;
		}
		if (currScore > bestScore)
		{
			bestScore = currScore;
			m_physicalDevice = device;
		}
	}
	assert(m_physicalDevice != VK_NULL_HANDLE);
}