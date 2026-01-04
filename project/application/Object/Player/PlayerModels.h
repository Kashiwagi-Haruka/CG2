#pragma once
#include "Object3d.h"
#include "Transform.h"
#include "Camera.h"
#include <memory>
class PlayerModels {

	enum State { 
		idle,
		walk,
		attack1,
		attack2,
		attack3,
		attack4,
		fallingAttack,
		skillAttack,
		damage,

	}state_;

	std::unique_ptr<Object3d> head_;
	std::unique_ptr<Object3d> armR_;
	std::unique_ptr<Object3d> armL_;
	std::unique_ptr<Object3d> legR_;
	std::unique_ptr<Object3d> legL_;

	Transform headT_;
	Transform armRT_;
	Transform armLT_;
	Transform legRT_;
	Transform legLT_;

	Transform player_;
	Camera* camera_;

public:
	PlayerModels();
	~PlayerModels();
	void SetCamera(Camera* camera) { camera_ = camera; };
	void SetPlayerTransform(Transform player) { player_ = player; };
	void Initialize();
	void Update();
	void Draw();

};
