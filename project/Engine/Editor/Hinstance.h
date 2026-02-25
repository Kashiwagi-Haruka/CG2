#pragma once

#include <string>
#include <vector>

#include "Transform.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

class Object3d;
class Primitive;

class Hinstance {
public:
	static Hinstance* GetInstance();

	void RegisterObject3d(Object3d* object);
	void UnregisterObject3d(Object3d* object);
	void RegisterPrimitive(Primitive* primitive);
	void UnregisterPrimitive(Primitive* primitive);
	bool HasRegisteredObjects() const;
	void DrawObjectEditors();
	void SetPlayMode(bool isPlaying);
	bool IsPlayMode() const { return isPlaying_; }
	bool LoadObjectEditorsFromJsonIfExists(const std::string& filePath);

private:
	void DrawSceneSelector();
	void DrawGridEditor();
	std::string GetSceneScopedEditorFilePath(const std::string& defaultFilePath) const;
	void ResetForSceneChange();

	struct EditorMaterial {
		Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
		bool enableLighting = true;
		float shininess = 40.0f;
		float environmentCoefficient = 0.0f;
		bool grayscaleEnabled = false;
		bool sepiaEnabled = false;
		float distortionStrength = 0.0f;
		float distortionFalloff = 1.0f;
		Vector3 uvScale = {1.0f, 1.0f, 1.0f};
		Vector3 uvRotate = {0.0f, 0.0f, 0.0f};
		Vector3 uvTranslate = {0.0f, 0.0f, 0.0f};
		Vector2 uvAnchor = {0.0f, 0.0f};
	};

	bool SaveObjectEditorsToJson(const std::string& filePath) const;
	bool LoadObjectEditorsFromJson(const std::string& filePath);

	std::vector<Object3d*> objects_;
	std::vector<std::string> objectNames_;
	std::vector<Transform> editorTransforms_;
	std::vector<EditorMaterial> editorMaterials_;

	std::vector<Primitive*> primitives_;
	std::vector<std::string> primitiveNames_;
	std::vector<Transform> primitiveEditorTransforms_;
	std::vector<EditorMaterial> primitiveEditorMaterials_;
	std::string saveStatusMessage_;
	bool hasUnsavedChanges_ = false;
	bool isPlaying_ = false;
	std::string loadedSceneName_;
	bool hasLoadedForCurrentScene_ = false;
	bool enableGridSnap_ = true;
	float gridSnapSpacing_ = 1.0f;
};