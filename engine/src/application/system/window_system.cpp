#include "window_system.h"
#include <application/engine.h>
#include <application/managers/window_manager.h>

//-- include events header

WindowSystem::WindowSystem(EngineContext& context, WindowInfo&& info) noexcept
	: m_context(context)
	, m_info(std::move(info))
{
	auto res = glfwInit();
	assert(res);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	auto& winManager = m_context.m_managerHolder.getManager<WindowManager>();
	winManager.m_window = glfwCreateWindow(m_info.m_width
		, m_info.m_height
		, m_info.m_windowName.c_str()
		, nullptr
		, nullptr);

	glfwSetWindowUserPointer(winManager.m_window, &m_info.m_handler);

	glfwSetWindowCloseCallback(winManager.m_window, [](GLFWwindow* win)
		{
			EventsHandler* eventHandler = (EventsHandler*)glfwGetWindowUserPointer(win);
		});

	glfwSetFramebufferSizeCallback(winManager.m_window, [](GLFWwindow* window, int width, int height)
		{
			EventsHandler* eventHandler = (EventsHandler*)glfwGetWindowUserPointer(window);
		});

	glfwSetKeyCallback(winManager.m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (key == GLFW_KEY_UP && action == GLFW_PRESS)
			{
			}
		});
}

WindowSystem::~WindowSystem() noexcept
{
	auto& winManager = m_context.m_managerHolder.getManager<WindowManager>();
	
	if (winManager.m_window != nullptr)
	{
		glfwDestroyWindow(winManager.m_window);
		glfwTerminate();
	}
}

void WindowSystem::update(float dt)
{
	glfwPollEvents();
}
