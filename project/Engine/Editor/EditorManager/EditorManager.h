#pragma once

#include <string>
#include <vector>

#include "Engine/Editor/EditorTool/Asset/EditorAsset.h"
#include "Engine/Editor/EditorTool/Grid/EditorGrid.h"
#include "Engine/Editor/EditorTool/Inspector/Inspector.h"
#include "Engine/Editor/EditorTool/ToolBar/ToolBar.h"
class Hierarchy;

class EditorManager {
public:
	static EditorManager* GetInstance();

	void Finalize();
	void DrawObjectEditors();
	void DrawAssetWindow();
	void DrawEditorGridLines();
	bool HasRegisteredObjects() const;
	ToolBar::Result DrawToolBar(bool isPlaying, bool isPaused, bool hasUnsavedChanges, bool canUndo, bool canRedo, bool& showGridMenu, EditorGrid::Settings& gridSettings) const;
	std::vector<EditorGridLine> CreateGridLines(int halfLineCount = 50, float spacing = 1.0f, float lineWidth = 1.0f) const;
	bool DrawObjectInspector(
	    size_t index, std::string& objectName, Transform& transform, InspectorMaterial& material, bool isPlaying, bool& transformChanged, bool& materialChanged, bool& nameChanged) const;
	bool DrawPrimitiveInspector(
	    size_t index, std::string& primitiveName, Transform& transform, InspectorMaterial& material, bool isPlaying, bool& transformChanged, bool& materialChanged, bool& nameChanged) const;

private:
	EditorManager();

	Hierarchy* hierarchy_ = nullptr;
	EditorAsset editorAsset_{};
};