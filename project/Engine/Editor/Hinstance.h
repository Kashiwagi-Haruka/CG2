#pragma once

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
	std::vector<Object3d*> objects_;
};