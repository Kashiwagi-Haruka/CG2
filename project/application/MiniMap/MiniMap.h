#pragma once
#include <string>
#include "Vector3.h"
#include "Vector4.h"
#include <memory>
class Object3d;
class Camera;
class MiniMap {
	
	std::unique_ptr<Camera> camera;

public:
	void AddObject(std::string Name, Object3d* object,Vector4 color);

	MiniMap();

	void Initialize();
	void Update();
	void Draw();
	void SetPlayerTranslate(Vector3 translate);

};
