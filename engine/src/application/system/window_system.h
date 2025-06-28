#pragma once

#include <application/core/system_interface.h>
#include <GLFW/glfw3.h>

struct EngineContext;

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
	WindowSystem(EngineContext& context, WindowInfo&& info) noexcept;
	~WindowSystem() noexcept;

	void	update(float dt);

private:
	EngineContext&	m_context;
	WindowInfo		m_info;
};