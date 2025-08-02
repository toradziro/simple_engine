#pragma once

#include <application/editor/editor_context.h>

class EntityPanel
{
public:
	EntityPanel() = default;
	EntityPanel(std::shared_ptr<EditorContext> context) : m_editorContext(context) {}
	void update();

private:
	std::shared_ptr<EditorContext>	m_editorContext;
};