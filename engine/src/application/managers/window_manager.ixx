export module window_manager;

import <glfw/glfw3.h>;

export struct WindowManager
{
public:
	WindowManager() = default;
	GLFWwindow* window()
	{
		return m_window;
	}

	GLFWwindow* m_window = nullptr;
};
