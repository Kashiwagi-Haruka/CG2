#pragma once
#include <string>
#include "Vector3.h"
class Object3d;
class Camera;
class MiniMap {
	
	Camera* camera;

public:
	void AddObject(std::string Name, Object3d* object);

	MiniMap();

	void Initialize();
	void Update();
	void Draw();
	void SetPlayerTranslate(Vector3 translate);

};
