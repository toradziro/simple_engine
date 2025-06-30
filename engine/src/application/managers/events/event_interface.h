#pragma once

#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <cassert>
#include <algorithm>
#include <ranges>
#include <concepts>

template<typename Event>
std::string eventId()
{
	return typeid(Event).name();
}

enum class EventCategory : uint8_t
{
	SystemEvent,	//-- Like resizing/closing window
	MouseEvent,
	KeeEvent
};

class Event
{
private:
	struct IEvent
	{
		virtual ~IEvent() = default;
		virtual std::string eventId() const = 0;
		virtual bool isHandeled() const = 0;
		virtual void setHandeled() = 0;
	};

	template <typename T>
	struct EventObject final : IEvent
	{
		template<typename... Args>
		EventObject(Args&&... args) : m_event(std::forward<Args>(args)...) {}

		virtual std::string eventId() const override
		{
			return ::eventId<T>();
		}

		virtual bool isHandeled() const override
		{
			return m_isHandeled;
		}

		virtual void setHandeled() override
		{
			m_isHandeled = true;
		}

		T		m_event;
		bool	m_isHandeled;
	};

public:
	template <typename T, typename... Args>
	explicit Event(std::in_place_type_t<T>, Args&&... args)
		: m_eventObject(std::make_unique<EventObject<T>>(std::forward<Args>(args)...))
	{}

	template <typename T>
	T& getUnderlyingEvent()
	{
		auto eventObject = static_cast<EventObject<T>*>(m_eventObject.get());
		return eventObject->m_event;
	}

	std::string eventId() const
	{
		return m_eventObject->eventId();
	}

private:
	std::unique_ptr<IEvent> m_eventObject;
};

struct EventHolder
{
	template<typename EventType, typename... Args>
	void addEvent(Args&&... args)
	{
		m_events.insert({
				eventId<EventType>(),
				Event{ std::in_place_type<EventType>, std::forward<Args>(args)... }
			});
	}

	template<typename T>
	T& getEvent()
	{
		assert(m_events.count(eventId<T>()));
		return m_events[eventId<T>()].getUnderlyingEvent<T>();
	}

	template<typename T>
	bool hasEvent() const
	{
		return m_events.count(eventId<T>()) && !isHandeled<T>();
	}

	template<typename T>
	bool isHandeled() const
	{
		return m_events[eventId<T>()].isHandeled();
	}

	template<typename T>
	void setHandeled()
	{
		m_events[eventId<T>()].setHandeled();
	}

	void clear()
	{
		m_events.clear();
	}

private:
	std::unordered_map<std::string, Event> m_events;
};
