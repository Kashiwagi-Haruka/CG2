#pragma once
#include <memory>
#include "Mesh/Object3d/Object3d.h"
#include "Transform.h"
class Camera;
class Enemy {

	std::unique_ptr<Object3d> obj_ = nullptr;
	Transform transform_;

public:
	Enemy();
	void Initialize();
	void SetCamera(Camera* camera);
	void Update();
	void Draw();


};
