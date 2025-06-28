#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <renderer/renderer.h>
#include <application/engine.h>
#include "utils.h"

int main(int argc, char** argv)
{
	Engine e{};
	e.run();
}