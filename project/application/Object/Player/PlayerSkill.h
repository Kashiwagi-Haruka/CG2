#pragma once
#include "Object3d.h"
#include <memory>
#include "Transform.h"
#include "Camera.h"
class PlayerSkill {

	private:

	std::unique_ptr<Object3d> debugBox_;
	Transform transform_;
	Camera* camera_ = nullptr;

	bool isSkillAttack = false;

	public:

		PlayerSkill();
	    void Initialize();
	    void Update(const Transform& playerTransform);
	    void Draw();
	    void SetCamera(Camera* camera) { camera_ = camera; }
	    void StartAttack();
};
