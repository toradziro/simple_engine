#include "utils.h"

#include <iostream>
#include <format>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

void checkLibs()
{
	// glm added check
	{
		glm::vec2 vec = { 1.0f, 10.0f };
		std::cout << std::format("{} {}", vec.x, vec.y) << std::endl;
	}

	// vulkan check
	{
		uint32_t version = 0;
		VkResult result = vkEnumerateInstanceVersion(&version);

		if (result == VK_SUCCESS)
		{
			uint32_t major = VK_VERSION_MAJOR(version);
			uint32_t minor = VK_VERSION_MINOR(version);
			uint32_t patch = VK_VERSION_PATCH(version);

			std::cout << "Vulkan API Version: " << major << "." << minor << "." << patch << std::endl;
		}
		else
		{
			std::cerr << "Failed to enumerate Vulkan version!" << std::endl;
		}

		uint32_t extentionsSupportedCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extentionsSupportedCount, nullptr);
		std::cout << std::format("Extentions conunt: {}\n", extentionsSupportedCount);
	}

	//-- glfw check
	{
		const char* version = glfwGetVersionString();
		std::cout << "GLFW Version: " << version << std::endl;
	}
}