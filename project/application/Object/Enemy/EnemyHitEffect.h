#pragma once
#include <memory>
#include "Object3d.h"
#include <vector>
#include "Transform.h"
class EnemyHitEffect {

	std::unique_ptr<Object3d> hitEffect_;
	Transform hitTransform_;
	Camera* camera_ = nullptr;
	Transform enemyTransform_;

	public:
	void Initialize();
	    void SetCamera(Camera* camera) { camera_ = camera; };
	void SetPosition(Transform transform) { enemyTransform_ = transform; };
	void Update();
	void Draw();


};
