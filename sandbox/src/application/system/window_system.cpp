#include "window_system.h"

//-- include events header


WindowSystem::WindowSystem(WindowInfo&& info) : m_info(std::move(info))
{
	auto res = glfwInit();
	assert(res);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(m_info.m_width
		, m_info.m_height
		, m_info.m_windowName.c_str()
		, nullptr
		, nullptr);

	glfwSetWindowUserPointer(m_window, &m_info.m_handler);

	glfwSetWindowCloseCallback(m_window, [](GLFWwindow* win)
		{
			EventsHandler* eventHandler = (EventsHandler*)glfwGetWindowUserPointer(win);
		});

	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
		{
			EventsHandler* eventHandler = (EventsHandler*)glfwGetWindowUserPointer(window);
		});

	glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (key == GLFW_KEY_UP && action == GLFW_PRESS)
			{
			}
		});
}

WindowSystem::WindowSystem(WindowSystem&& winSystem)
{
	m_info = std::move(winSystem.m_info);
	m_window = winSystem.m_window;

	winSystem.m_window = nullptr;
}

WindowSystem::~WindowSystem()
{
	if (m_window != nullptr)
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}
}

void WindowSystem::update(float dt)
{
	glfwPollEvents();
}

void WindowSystem::init()
{}

void WindowSystem::shutdown()
{}
