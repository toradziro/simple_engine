#pragma once

#include <application/core/system_interface.h>
#include <GLFW/glfw3.h>

struct EventsHandler
{

};

struct WindowInfo
{
	EventsHandler	m_handler;
	std::string		m_windowName;
	int				m_width;
	int				m_height;
};

class WindowSystem
{
public:
	WindowSystem(WindowInfo&& info);
	WindowSystem(WindowSystem&& winSystem);
	~WindowSystem();

	void	update(float dt);
	void	init();
	void	shutdown();

private:
	WindowInfo	m_info;
	GLFWwindow* m_window = nullptr;
};