#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <renderer/renderer.h>
#include "utils.h"

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

	Renderer renderer;
	renderer.init(win);

	while (!shouldClouseWindow)
	{
		glfwPollEvents();
		renderer.update(0.0f);
		glfwSwapBuffers(win);
	}

	glfwDestroyWindow(win);

	glfwTerminate();
}