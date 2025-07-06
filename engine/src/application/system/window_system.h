#pragma once

#include <application/managers/events/event_interface.h>
#include <GLFW/glfw3.h>
#include <functional>

struct EngineContext;

struct WindowInfo
{
	std::string					m_windowName;
	int							m_width;
	int							m_height;
	std::function<void(Event&)>	m_eventCallback;
};

class WindowSystem
{
public:
	WindowSystem(EngineContext& context, WindowInfo&& info) noexcept;
	~WindowSystem() noexcept;

	void	update(float dt);
	void	onEvent(Event& event) const;

private:
	EngineContext&	m_context;
	WindowInfo		m_info;
};