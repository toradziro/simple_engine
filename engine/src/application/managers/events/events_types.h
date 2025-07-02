#pragma once

//-- Application events
struct WindowCloseEvent
{
};

//-- Key events
struct WindowResizeEvent
{
	int		m_width;
	int		m_height;
};

struct KeyPressedEvent
{
	int		m_repeatCount;
	int		m_keyCode;
};

struct KeyReleasedEvent
{
	int		m_keyCode;
};

//-- Mouse Events
struct MouseButtonPressedEvent
{
	int		m_buttonCode;
};

struct MouseButtonReleasedEvent
{
	int		m_buttonCode;
};

struct MouseMovedEvent
{
	float	m_mouseX;
	float	m_mouseY;
};

struct MouseScrolledEvent
{
	float	m_offsetX;
	float	m_offsetY;
};
