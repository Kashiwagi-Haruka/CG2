#pragma once
#include <memory>
#include "Object3d.h"
#include "Transform.h"
class PlayerSpecialAttack {

	std::unique_ptr<Object3d> debugBox_;
	Camera* camera_ = nullptr;
	Transform transform_;

public:
	PlayerSpecialAttack();
	void Initialize();
	void Update(const Transform& playerTransform);
	void Draw();
	void SetCamera(Camera* camera) { camera_ = camera; }

};
