#include "renderer_manager.h"

//-------------------------------------------------------------------------------------------------
void RendererManager::addSpriteToDrawList(SpriteInfo spriteInfo)
{
	m_sprites.push_back(spriteInfo);
}

//-------------------------------------------------------------------------------------------------
void RendererManager::addImGuiDrawCallback(ImGuiDrawCallback imGuiUpdateUi)
{
	m_imGuiUpdatesUi.emplace_back(imGuiUpdateUi);
}
