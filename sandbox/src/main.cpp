#include <iostream>
#include <format>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

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

int main(int argc, char** argv)
{
	checkLibs();

	assert(glfwInit());

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* win = glfwCreateWindow(1200, 800, "vulkan_try", nullptr, nullptr);
	bool shouldClouseWindow = false;
	glfwSetWindowUserPointer(win, &shouldClouseWindow);

	glfwSetWindowCloseCallback(win, [](GLFWwindow* win)
		{
			bool* shouldClose = (bool*)glfwGetWindowUserPointer(win);
			assert(shouldClose != nullptr);
			*shouldClose = true;
		});

	while (!shouldClouseWindow)
	{
		glfwPollEvents();
		glfwSwapBuffers(win);
	}

	glfwDestroyWindow(win);

	glfwTerminate();
}