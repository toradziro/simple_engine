export module event_interface;

import <memory>;
import <string>;
import <cassert>;
import <algorithm>;
import <ranges>;
import <concepts>;
import <functional>;
import <typeinfo>;

//-------------------------------------------------------------------------------------------------
template<typename Event>
std::string eventId()
{
	return typeid(Event).name();
}

//-------------------------------------------------------------------------------------------------
export class Event
{
public:
	//-------------------------------------------------------------------------------------------------
	template <typename T>
	Event(T event)
		: m_eventObject(std::make_unique<EventObject<T>>(std::move(event)))
	{}

	//-------------------------------------------------------------------------------------------------
	template <typename T>
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
	const std::string& eventId() const
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

		virtual bool				isHandeled() const = 0;
		virtual void				setHandeled() = 0;
		virtual const std::string&	eventId() const = 0;
	};

	//-------------------------------------------------------------------------------------------------
	template <typename T>
	struct EventObject final : IEvent
	{
		//-------------------------------------------------------------------------------------------------
		EventObject(T&& event) : m_event(std::move(event)), m_eventId(::eventId<T>()) {}

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
		virtual const std::string& eventId() const override
		{
			return m_eventId;
		}

		T					m_event;
		bool				m_isHandeled = false;
		const std::string	m_eventId;
	};

private:
	std::unique_ptr<IEvent> m_eventObject;
};

//-------------------------------------------------------------------------------------------------
export template<typename T>
bool eventTypeCheck(Event& event)
{
	return event.eventId() == eventId<T>() && !event.isHandeled();
}

//-------------------------------------------------------------------------------------------------
//-- Events Types
//-- Application events
export struct WindowCloseEvent
{
};

//-- Key events
export struct WindowResizeEvent
{
	int		m_width;
	int		m_height;
};

//-------------------------------------------------------------------------------------------------
export struct KeyPressedEvent
{
	int		m_keyCode;
	int		m_repeatCount;
};

//-------------------------------------------------------------------------------------------------
export struct KeyReleasedEvent
{
	int		m_keyCode;
};

//-------------------------------------------------------------------------------------------------
//-- Mouse Events
export struct MouseButtonPressedEvent
{
	int		m_buttonCode;
};

//-------------------------------------------------------------------------------------------------
export struct MouseButtonReleasedEvent
{
	int		m_buttonCode;
};

//-------------------------------------------------------------------------------------------------
export struct MouseMovedEvent
{
	float	m_mouseX;
	float	m_mouseY;
};

//-------------------------------------------------------------------------------------------------
export struct MouseScrolledEvent
{
	float	m_offsetX;
	float	m_offsetY;
};
