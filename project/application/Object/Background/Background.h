#pragma once
#include "Camera.h"
#include "Object3d.h"

class Background {

	Object3d* object_ = nullptr;
	Object3d* object2_ = nullptr;
	Camera* camera_ = nullptr;
	Vector3 translate;
	Vector3 translate2_;
	float initPosX;

	public:

	Background();
	~Background();
	void Initialize();
	void Update(Vector3 vector3);
	void Draw();
	void SetCamera(Camera* camera);
	void SetPosition(Vector3 vector3);
};
