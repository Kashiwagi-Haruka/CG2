#pragma once

#include <string>
#include <vector>

#include "Transform.h"

class Object3d;

class Hinstance {
public:
	static Hinstance* GetInstance();

	void RegisterObject3d(Object3d* object);
	void UnregisterObject3d(Object3d* object);
	bool HasRegisteredObjects() const;
	void DrawObjectEditors();
	void SetPlayMode(bool isPlaying);
	bool IsPlayMode() const { return isPlaying_; }

private:
	bool SaveObjectEditorsToJson(const std::string& filePath) const;
	bool LoadObjectEditorsFromJson(const std::string& filePath);

	std::vector<Object3d*> objects_;
	std::vector<Transform> editorTransforms_;
	std::string saveStatusMessage_;
	bool isPlaying_ = false;
};