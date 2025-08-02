#pragma once

#include <memory>
#include <string>
#include <cassert>
#include <algorithm>
#include <ranges>
#include <concepts>
#include <functional>
#include <typeinfo>

//-------------------------------------------------------------------------------------------------
template<typename Event>
std::string eventId()
{
	return typeid(Event).name();
}

//-------------------------------------------------------------------------------------------------
class Event
{
public:
	//-------------------------------------------------------------------------------------------------
	template<typename T>
	Event(T event)
		: m_eventObject(std::make_unique<EventObject<T>>(std::move(event))) {}

	//-------------------------------------------------------------------------------------------------
	template<typename T>
	T& getUnderlyingEvent()
	{
		auto eventObject = static_cast<EventObject<T>*>(m_eventObject.get());
		return eventObject->m_event;
	}

	//-------------------------------------------------------------------------------------------------
	void setHandeled()
	{
		m_eventObject->setHandeled();
	}

private:
	//-------------------------------------------------------------------------------------------------
	template<typename T>
	friend bool eventTypeCheck(Event& event);

	//-------------------------------------------------------------------------------------------------
	std::string_view eventId() const
	{
		return m_eventObject->eventId();
	}

	//-------------------------------------------------------------------------------------------------
	bool isHandeled() const
	{
		return m_eventObject->isHandeled();
	}

private:
	//-------------------------------------------------------------------------------------------------
	struct IEvent
	{
		virtual ~IEvent() = default;

		virtual bool isHandeled() const = 0;

		virtual void setHandeled() = 0;

		virtual std::string_view eventId() const = 0;
	};

	//-------------------------------------------------------------------------------------------------
	template<typename T>
	struct EventObject final : IEvent
	{
		//-------------------------------------------------------------------------------------------------
		EventObject(T&& event) : m_event(std::move(event))
		                       , m_eventId(::eventId<T>()) {}

		//-------------------------------------------------------------------------------------------------
		virtual bool isHandeled() const override
		{
			return m_isHandeled;
		}

		//-------------------------------------------------------------------------------------------------
		virtual void setHandeled() override
		{
			m_isHandeled = true;
		}

		//-------------------------------------------------------------------------------------------------
		virtual std::string_view eventId() const override
		{
			return m_eventId;
		}

		T                 m_event;
		bool              m_isHandeled = false;
		const std::string m_eventId;
	};

private:
	std::unique_ptr<IEvent> m_eventObject;
};

//-------------------------------------------------------------------------------------------------
template<typename T>
bool eventTypeCheck(Event& event)
{
	return event.eventId() == eventId<T>() && !event.isHandeled();
}

//-------------------------------------------------------------------------------------------------
//-- Events Types
//-- Application events
struct WindowCloseEvent {};

//-- Key events
struct WindowResizeEvent
{
	int m_width;
	int m_height;
};

//-------------------------------------------------------------------------------------------------
struct KeyPressedEvent
{
	int m_keyCode;
	int m_repeatCount;
};

//-------------------------------------------------------------------------------------------------
struct KeyReleasedEvent
{
	int m_keyCode;
};

//-------------------------------------------------------------------------------------------------
//-- Mouse Events
struct MouseButtonPressedEvent
{
	int m_buttonCode;
};

//-------------------------------------------------------------------------------------------------
struct MouseButtonReleasedEvent
{
	int m_buttonCode;
};

//-------------------------------------------------------------------------------------------------
struct MouseMovedEvent
{
	float m_mouseX;
	float m_mouseY;
};

//-------------------------------------------------------------------------------------------------
struct MouseScrolledEvent
{
	float m_offsetX;
	float m_offsetY;
};
