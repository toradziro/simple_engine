#pragma once

#include <functional>
#include <string>
#include <cassert>
#include <memory>

#include <GLFW/glfw3.h>

struct Event;
struct EngineContext;

//-------------------------------------------------------------------------------------------------
struct WindowInfo
{
	std::string					m_windowName;
	int							m_width;
	int							m_height;
	std::function<void(Event&)>	m_eventCallback;
};

//-------------------------------------------------------------------------------------------------
class WindowSystem
{
public:
	WindowSystem(std::shared_ptr<EngineContext> context, WindowInfo&& info) noexcept;
	~WindowSystem() noexcept;

	void	update(float dt);
	void	onEvent(Event& event) const
	{
	}

private:
	std::shared_ptr<EngineContext>	m_context;
	WindowInfo		m_info;
};