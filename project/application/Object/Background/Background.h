#pragma once
#include "Camera.h"
#include "Object3d.h"
#include <memory>

class Background {

	std::unique_ptr<Object3d> object_ = nullptr;
	std::unique_ptr<Object3d> object2_ = nullptr;

	Camera* camera_ = nullptr;
	Vector3 translate;
	Vector3 translate2_;
	float initPosX;

public:
	Background();
	~Background() = default; 
	void Initialize();
	void Update(Vector3 vector3);
	void Draw();
	void SetCamera(Camera* camera);
	void SetPosition(Vector3 vector3);
};
