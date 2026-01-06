#pragma once
#include "Object3d.h"
#include "Transform.h"
#include <memory>
#include <vector>
class PlayerSpecialAttack {

	std::unique_ptr<std::vector<Object3d>> iceFlowers_;
	std::vector<Transform> iceFlowerTransforms_;
	std::unique_ptr<Object3d> debugBox_;
	Camera* camera_ = nullptr;
	Transform transform_;
	float radius = 3.0f;
	float fallSpeed_ = 0.25f;
	float startHeight_ = 6.0f;
	bool isSpecialEnd_;
	int specialTime_ = 0;
	int specialTimeMax_ = 60;

	void EnsureIceFlowerCount(int count);

public:
	PlayerSpecialAttack();
	void Initialize();
	void Update(const Transform& playerTransform);
	void Draw();
	void SetCamera(Camera* camera) { camera_ = camera; }
	void StartAttack(const Transform& playerTransform, int iceCount);
	bool IsSpecialEnd() { return isSpecialEnd_; }
};