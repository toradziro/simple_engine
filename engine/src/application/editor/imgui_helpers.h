#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <imgui.h>
#include <format>

void drawVec3Prop(glm::vec3& val, std::string_view propName);