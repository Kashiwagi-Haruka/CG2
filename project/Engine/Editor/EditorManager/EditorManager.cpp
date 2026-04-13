#include "EditorManager.h"

#include "Engine/Editor/EditorTool/Hierarchy/Hierarchy.h"

EditorManager* EditorManager::GetInstance() {
	static EditorManager instance;
	return &instance;
}

EditorManager::EditorManager() : hierarchy_(Hierarchy::GetInstance()) {}

void EditorManager::Finalize() {
	if (hierarchy_) {
		hierarchy_->Finalize();
	}
}

void EditorManager::DrawObjectEditors() {
	if (hierarchy_) {
		hierarchy_->DrawObjectEditors();
	}
}

void EditorManager::DrawAssetWindow() { editorAsset_.EditorDraw(); }

void EditorManager::DrawEditorGridLines() {
	if (hierarchy_) {
		hierarchy_->DrawEditorGridLines();
	}
}

bool EditorManager::HasRegisteredObjects() const { return hierarchy_ && hierarchy_->HasRegisteredObjects(); }

ToolBar::Result EditorManager::DrawToolBar(bool isPlaying, bool isPaused, bool hasUnsavedChanges, bool canUndo, bool canRedo, bool& showGridMenu, EditorGrid::Settings& gridSettings) const {
	return ToolBar::Draw(isPlaying, isPaused, hasUnsavedChanges, canUndo, canRedo, showGridMenu, gridSettings);
}

std::vector<EditorGridLine> EditorManager::CreateGridLines(int halfLineCount, float spacing, float lineWidth) const { return EditorGrid::CreateLines(halfLineCount, spacing, lineWidth); }

bool EditorManager::DrawObjectInspector(
    size_t index, std::string& objectName, Transform& transform, InspectorMaterial& material, bool isPlaying, bool& transformChanged, bool& materialChanged, bool& nameChanged) const {
	return Inspector::DrawObjectInspector(index, objectName, transform, material, isPlaying, transformChanged, materialChanged, nameChanged);
}

bool EditorManager::DrawPrimitiveInspector(
    size_t index, std::string& primitiveName, Transform& transform, InspectorMaterial& material, bool isPlaying, bool& transformChanged, bool& materialChanged, bool& nameChanged) const {
	return Inspector::DrawPrimitiveInspector(index, primitiveName, transform, material, isPlaying, transformChanged, materialChanged, nameChanged);
}