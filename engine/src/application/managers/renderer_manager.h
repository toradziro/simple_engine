#pragma once

#include <vector>
#include <string>
#include <array>
#include <functional>

#include <glm/glm.hpp>

//-------------------------------------------------------------------------------------------------
struct SpriteInfo
{
	glm::vec3   m_position;
	std::string m_texturePath;
};

//-------------------------------------------------------------------------------------------------
struct RendererManager
{
	using ImGuiDrawCallback = std::function<void()>;

	//-------------------------------------------------------------------------------------------------
	void addSpriteToDrawList(const SpriteInfo& spriteInfo);

	//-------------------------------------------------------------------------------------------------
	void addImGuiDrawCallback(ImGuiDrawCallback imGuiUpdateUi);

	//-- User notation object
	std::vector<SpriteInfo>        m_sprites;
	std::vector<ImGuiDrawCallback> m_imGuiUpdatesUi;
};
