#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Transform.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include "Engine/Editor/EditorData/Audio/EditorAudio.h"
#include "Engine/Editor/EditorData/Camera/EditorCamera.h"
#include "Engine/Editor/EditorTool/Grid/EditorGrid.h"
#include "Engine/Editor/EditorTool/Inspector/Inspector.h"
#include "Engine/Editor/EditorData/Light/EditorLight.h"

class Object3d;
class Primitive;
class Camera;

class Hierarchy {
public:
	static Hierarchy* GetInstance();

	void RegisterObject3d(Object3d* object);
	void UnregisterObject3d(Object3d* object);
	void RegisterPrimitive(Primitive* primitive);
	void UnregisterPrimitive(Primitive* primitive);
	bool HasRegisteredObjects() const;
	void DrawObjectEditors();
	void DrawEditorGridLines();
	void UpdateEditorPreview();
	void SetPlayMode(bool isPlaying);
	bool IsPlayMode() const { return isPlaying_; }
	bool IsPaused() const { return isPaused_; }
	bool IsEditorPreviewActive() const;
	bool LoadObjectEditorsFromJsonIfExists(const std::string& filePath);
	void Finalize();

private:
	struct EditorSnapshot {
		std::vector<Transform> objectTransforms;
		std::vector<InspectorMaterial> objectMaterials;
		std::vector<std::string> objectNames;
		std::vector<Transform> primitiveTransforms;
		std::vector<InspectorMaterial> primitiveMaterials;
		std::vector<std::string> primitiveNames;
	};

	void DrawSceneSelector();
	void DrawLightEditor();
	void DrawSelectionBoxEditor();
	void DrawCameraEditor();
	void DrawCameraBillboards();
	void SyncSelectionBoxToTarget();
	Transform GetSelectedTransform() const;
	bool IsObjectSelected() const;
	std::string GetSceneScopedEditorFilePath(const std::string& defaultFilePath) const;
	void ResetForSceneChange();
	void ApplyEditorSnapshot(const EditorSnapshot& snapshot);
	EditorSnapshot CreateCurrentSnapshot() const;
	bool ResetToLoadedSnapshot();
	void UndoEditorChange();
	void RedoEditorChange();

	bool SaveObjectEditorsToJson(const std::string& filePath) const;
	bool LoadObjectEditorsFromJson(const std::string& filePath);

	std::vector<EditorSnapshot> undoStack_;
	std::vector<EditorSnapshot> redoStack_;
	EditorSnapshot loadedSnapshot_{};
	bool hasLoadedSnapshot_ = false;
	std::string loadedSnapshotFilePath_;

	std::vector<Object3d*> objects_;
	std::vector<std::string> objectNames_;
	std::vector<Transform> editorTransforms_;
	std::vector<InspectorMaterial> editorMaterials_;

	std::vector<Primitive*> primitives_;
	std::vector<std::string> primitiveNames_;
	std::vector<Transform> primitiveEditorTransforms_;
	std::vector<InspectorMaterial> primitiveEditorMaterials_;
	std::string saveStatusMessage_;

	bool showSelectionBox_ = true;
	std::unique_ptr<Primitive> selectionBoxPrimitive_;
	bool selectionBoxDirty_ = true;
	size_t selectedObjectIndex_ = 0;
	bool selectedIsPrimitive_ = false;

	bool hasUnsavedChanges_ = false;
	bool isPlaying_ = false;
	bool isPaused_ = false;
	std::string loadedSceneName_;
	bool hasLoadedForCurrentScene_ = false;
	EditorGrid::Settings gridSettings_{};
	bool showGridWindow_ = true;
	std::unique_ptr<Primitive> editorGridPlane_;
	EditorLight editorLight_{};
	EditorAudio editorAudio_{};
	EditorCamera editorCamera_{};

public:
	void RegisterCamera(Camera* camera);
	void UnregisterCamera(Camera* camera);
};