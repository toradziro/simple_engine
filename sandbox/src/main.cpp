#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <renderer/renderer.h>
#include "utils.h"

int main(int argc, char** argv)
{
	//checkLibs();

	auto res = glfwInit();
	assert(res);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* win = glfwCreateWindow(1200, 800, "vulkan_try", nullptr, nullptr);
	struct WinData
	{
		bool m_shouldClouseWindow = false;
		bool m_resizedWindow = false;
	};
	WinData data = {};
	glfwSetWindowUserPointer(win, &data);

	glfwSetWindowCloseCallback(win, [](GLFWwindow* win)
		{
			WinData* data = (WinData*)glfwGetWindowUserPointer(win);
			data->m_shouldClouseWindow = true;
		});

	glfwSetFramebufferSizeCallback(win, [](GLFWwindow* window, int width, int height)
		{
			WinData* data = (WinData*)glfwGetWindowUserPointer(window);
			data->m_resizedWindow = true;
		});

	Renderer renderer;
	renderer.init(win);

	while (!data.m_shouldClouseWindow)
	{
		glfwPollEvents();
		if (data.m_resizedWindow)
		{
			renderer.resizedWindow();
			data.m_resizedWindow = false;
		}
		renderer.update(0.0f);
	}

	glfwDestroyWindow(win);

	glfwTerminate();
}