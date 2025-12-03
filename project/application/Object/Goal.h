#pragma once
#include "Transform.h"
#include "Camera.h"
class Object3d;
class GameBase;
class Goal {

private:

	Transform transform_={{1,1,1},{0,0,0},{10,3,0}};
	Object3d* object3d_;
	Camera* camera_;

public:

	Goal();
	~Goal();
	void Initialize(Camera* camera);
	void Update();
	void Draw();
	Vector3 GetTranslate() { return transform_.translate; };
	void SetCamera(Camera* camera) { camera_ = camera; }
};
