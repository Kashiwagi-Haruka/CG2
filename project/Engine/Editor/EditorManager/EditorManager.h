#pragma once

#include <string>
#include <vector>

#include "Engine/Editor/Grid/EditorGrid.h"
#include "Engine/Editor/Inspector/Inspector.h"
#include "Engine/Editor/ToolBar/ToolBar.h"

class Hierarchy;

class EditorManager {
public:
	static EditorManager* GetInstance();

	void Finalize();
	void DrawObjectEditors();
	void DrawEditorGridLines();
	bool HasRegisteredObjects() const;
	ToolBar::Result DrawToolBar(bool isPlaying, bool hasUnsavedChanges, bool canUndo, bool canRedo) const;
	std::vector<EditorGridLine> CreateGridLines(int halfLineCount = 50, float spacing = 1.0f, float lineWidth = 1.0f) const;
	bool DrawObjectInspector(
	    size_t index, std::string& objectName, Transform& transform, InspectorMaterial& material, bool isPlaying, bool& transformChanged, bool& materialChanged, bool& nameChanged) const;
	bool DrawPrimitiveInspector(
	    size_t index, std::string& primitiveName, Transform& transform, InspectorMaterial& material, bool isPlaying, bool& transformChanged, bool& materialChanged, bool& nameChanged) const;

private:
	EditorManager();

	Hierarchy* hierarchy_ = nullptr;
};