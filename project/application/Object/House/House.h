#pragma once
#include "Camera.h"
#include "Object3d.h"
#include "Vector3.h"
#include <memory>
#include "Transform.h"
class House {

	std::unique_ptr<Object3d> object_;
	std::unique_ptr<Object3d> hpbar_;
	std::unique_ptr<Object3d> hpflame_;
	Vector3 position_ = {0, 1.5f, 0};
	int hp_ = 50;

	Transform hpBarT_;
	Transform hpFlameT_;

public:
	House();
	~House() = default;

	void Initialize(Camera* camera);
	void Update(Camera* camera);
	void Draw();

	Vector3 GetPosition() const { return position_; }
	Vector3 GetScale() const { return object_->GetTransform().scale; }
	int GetHP() const { return hp_; }
	void Damage(int amount) { hp_ -= amount; }
};
