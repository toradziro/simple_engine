#pragma once

#include <application/editor/editor_context.h>

class ScenePanel
{
public:
	ScenePanel(EditorContext& context) : m_editorContext(context) {}
	void update();

private:
	void drawContextMenu();
	void selectEntity(Entity e);
	void resetSelection();

private:
	EditorContext&	m_editorContext;
};