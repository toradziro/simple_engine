#include "renderer.h"

Renderer::Renderer(GLFWwindow* window)
{
	m_device.init(window);
}

void Renderer::beginFrame(float dt)
{
	m_device.beginFrame(dt);
}

void Renderer::endFrame()
{
	m_device.endFrame();
}

void Renderer::drawSprite(/*sprite info*/)
{
	//-- Draw sprite command
}

