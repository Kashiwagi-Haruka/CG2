#pragma once

#include "Engine/Camera/DebugCamera.h"
#include "Primitive/Primitive.h"
#include <memory>
#include <string>
#include <vector>

class Camera;
class Object3d;
class Primitive;

class EditorCamera {
public:
	void Reset();
	void DeactivatePreview();
	void UpdateEditorPreview(bool isActive, const std::vector<Object3d*>& objects, const std::vector<Primitive*>& primitives);
	void DrawCameraEditor();
	void DrawCameraBillboards(bool isPlaying);
	void RegisterCamera(Camera* camera);
	void UnregisterCamera(Camera* camera);

private:
	void LoadEditorData(Camera& camera);
	void SaveEditorData(const Camera& camera);

	DebugCamera editorPreviewCamera_{};
	bool isEditorPreviewCameraInitialized_ = false;
	bool wasEditorPreviewActiveLastFrame_ = false;
	std::vector<Camera*> cameras_;
	std::unique_ptr<Primitive> cameraBillboardPrimitive_;
	std::string editorStatusMessage_;
};