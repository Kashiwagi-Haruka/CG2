#pragma once
#include <memory>
#include "Mesh/Object3d/Object3d.h"
#include "Transform.h"
class Camera;
class Enemy {

	std::unique_ptr<Object3d> obj_ = nullptr;
	Transform transform_;

	enum class EnemyState {
		Idle,
		Walking,
		Break,
	} state_ = EnemyState::Idle;


public:
	Enemy();
	void Initialize();
	void SetCamera(Camera* camera);
	void Update();
	void Draw();


};
