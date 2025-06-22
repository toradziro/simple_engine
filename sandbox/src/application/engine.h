#pragma once

#include <application/core/system_interface.h>
#include <renderer/renderer.h>

class Engine
{
public:
	Engine();
	~Engine();

	void	run();

private:
	SystemHolder	m_systemHolder;

	//-- test
	SpriteInfo		m_firstSprite;
	SpriteInfo		m_secondSprite;
};