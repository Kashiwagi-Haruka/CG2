#pragma once

#include <string>
#include <vector>

class Object3d;

class Hinstance {
public:
	static Hinstance* GetInstance();

	void RegisterObject3d(Object3d* object);
	void UnregisterObject3d(Object3d* object);
	bool HasRegisteredObjects() const;
	void DrawObjectEditors();

private:
	bool SaveObjectEditorsToJson(const std::string& filePath) const;

	std::vector<Object3d*> objects_;
	std::string saveStatusMessage_;
};