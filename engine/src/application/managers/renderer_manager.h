#pragma once

#include <vector>
#include <string>
#include <array>
#include <glm/glm.hpp>

class RendererSystem;

struct VertexData
{
	glm::vec2	m_vertex;
	glm::vec3	m_color;
	glm::vec2	m_texCoord;
};

struct SpriteInfo
{
	std::array<VertexData, 4>	m_verticies;
	std::string					m_texturePath;
};

class RendererManager
{
public:
	void addSpriteToDrawList(const SpriteInfo& spriteInfo)
	{
		m_sprites.push_back(spriteInfo);
	}

private:
	friend class RendererSystem;

	//-- User notation object
	std::vector<SpriteInfo>				m_sprites;
};