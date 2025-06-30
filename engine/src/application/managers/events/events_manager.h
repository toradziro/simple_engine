#pragma once

#include <glfw/glfw3.h>
#include <unordered_set>
#include "event_interface.h"

class WindowSystem;

class EventsManager
{
public:
	template<typename T>
	bool hasEvent() const
	{
		return m_events.hasEvent<T>();
	}

	template<typename T>
	void setHandeled()
	{
		m_events.setHandeled();
	}

	template<typename T>
	T& getEvent()
	{
		m_events.getEvent<T>();
	}

private:
	friend class WindowSystem;

	void clear()
	{
		m_events.clear();
	}

	template<typename T, typename... Args>
	void addEvent(Args&&... args)
	{
		m_events.addEvent<T>(std::forward<Args>(args)...);
	}

private:
	EventHolder		m_events;
};