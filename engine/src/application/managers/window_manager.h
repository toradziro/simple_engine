#pragma once

#include <glfw/glfw3.h>

class WindowSystem;

class WindowManager
{
public:
	WindowManager() = default;
	GLFWwindow* window() { return m_window; }

private:
	friend WindowSystem;

	GLFWwindow* m_window = nullptr;
};
