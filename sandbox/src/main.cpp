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
		bool m_buttomUpPressed = false;
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

	Renderer renderer(win);
	int txtId = 0;
	renderer.setTexture("images/nyan_cat.png");

	glfwSetKeyCallback(win, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (key == GLFW_KEY_UP && action == GLFW_PRESS)
			{
				WinData* data = (WinData*)glfwGetWindowUserPointer(window);
				data->m_buttomUpPressed = true;
			}
		});

	const std::array<VertexData, 4> firstQuad = {
		VertexData{{-0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }},
		VertexData{{0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, { 1.0f, 0.0f }},
		VertexData{{0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, { 1.0f, 1.0f }},
		VertexData{{-0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, { 0.0f, 1.0f }}
	};
	const std::array<VertexData, 4> secondQuad = {
		VertexData{{0.25f, -0.5f}, {0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }},
		VertexData{{1.25f, -0.5f}, {0.0f, 0.0f, 0.0f}, { 1.0f, 0.0f }},
		VertexData{{1.25f, 0.5f}, {0.0f, 0.0f, 0.0f}, { 1.0f, 1.0f }},
		VertexData{{0.25f, 0.5f}, {0.0f, 0.0f, 0.0f}, { 0.0f, 1.0f }}
	};

	while (!data.m_shouldClouseWindow)
	{
		glfwPollEvents();
		if (data.m_resizedWindow)
		{
			renderer.resizedWindow();
			data.m_resizedWindow = false;
		}
		if (data.m_buttomUpPressed)
		{
			if (!(txtId % 3))
			{
				renderer.setTexture("images/gg2.png");
			}
			else if (!(txtId % 2))
			{
				renderer.setTexture("images/nyan_cat.png");
			}
			else
			{
				renderer.setTexture("images/e_v.png");
			}
			txtId++;
			data.m_buttomUpPressed = false;
		}
		renderer.beginFrame(0.0f);
		renderer.drawSprite({ firstQuad });
		renderer.drawSprite({ secondQuad });
		//-- drawSprite will got here
		renderer.endFrame();
	}

	glfwDestroyWindow(win);

	glfwTerminate();
}