#pragma once

#include "device.h"

class Renderer
{
public:
	Renderer(GLFWwindow* window);

	void beginFrame(float dt);
	void endFrame();
	void drawSprite(/*sprite info*/);
	void resizedWindow() { m_device.resizedWindow(); }

private:
	VkGraphicDevice	m_device;
};