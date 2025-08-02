#pragma once

#include <application/editor/editor_context.h>

class ScenePanel
{
public:
	ScenePanel() = default;
	ScenePanel(std::shared_ptr<EditorContext> context) : m_editorContext(context) {}
	void update();

private:
	void drawContextMenu();
	void selectEntity(Entity e);
	void resetSelection();

private:
	std::shared_ptr<EditorContext>	m_editorContext;
};