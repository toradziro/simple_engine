#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <cassert>
#include <algorithm>
#include <ranges>
#include <concepts>
#include <functional>

template<typename Event>
std::string eventId()
{
	return typeid(Event).name();
}

class Event
{
public:
	template <typename T>
	Event(T event)
		: m_eventObject(std::make_unique<EventObject<T>>(std::move(event)))
	{}

	template <typename T>
	T& getUnderlyingEvent()
	{
		auto eventObject = static_cast<EventObject<T>*>(m_eventObject.get());
		return eventObject->m_event;
	}

	const std::string& eventId() const
	{
		return m_eventObject->eventId();
	}

	void setHandeled()
	{
		m_eventObject->setHandeled();
	}

private:
	struct IEvent
	{
		virtual ~IEvent() = default;

		virtual bool				isHandeled() const = 0;
		virtual void				setHandeled() = 0;
		virtual const std::string&	eventId() const = 0;
	};

	template <typename T>
	struct EventObject final : IEvent
	{
		EventObject(T&& event) : m_event(std::move(event)), m_eventId(::eventId<T>()) {}

		virtual bool isHandeled() const override
		{
			return m_isHandeled;
		}

		virtual void setHandeled() override
		{
			m_isHandeled = true;
		}

		virtual const std::string& eventId() const override
		{
			return m_eventId;
		}

		T					m_event;
		bool				m_isHandeled;
		const std::string	m_eventId;
	};

private:
	std::unique_ptr<IEvent> m_eventObject;
};

class EventDispatcher
{
private:
	template<typename T>
	using EventFn = std::function<bool(T&)>;

public:
	EventDispatcher(Event& event) : m_event(event) {}

	template<typename T>
	bool dispatch(EventFn<T> handler)
	{
		if (m_event.eventId() == eventId<T>())
		{
			if (handler(m_event.getUnderlyingEvent()))
			{
				m_event.setHandeled();
			}
			return true;
		}
		return false;
	}

private:
	Event& m_event;
};