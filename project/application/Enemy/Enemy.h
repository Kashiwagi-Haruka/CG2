#pragma once
#include "Transform.h"
#include "Vector3.h"
class GameBase;
class Camera;
class Object3d;

class Enemy {

	int HP = 1;
	bool isAlive = true; // 生きているか
	bool isHit = false;//踏まれたか

	Vector3 direction_;
	Vector3 velocity_;
	Transform transform_;

	float maxSpeed_ = 0.1f;


	Object3d* object_ = nullptr;
	Camera* camera_ = nullptr;

public:

	~Enemy();
	void Initialize(GameBase* gameBase,Camera* camera);
	void Update(GameBase* gameBase);
	void Draw(GameBase* gameBase);
	void SetCamera(Camera* camera) { camera_ = camera;}
	void BulletCollision();
};
