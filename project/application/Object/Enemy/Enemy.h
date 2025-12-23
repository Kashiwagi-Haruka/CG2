#pragma once
#include "EnemyStun.h"
#include "Transform.h"
#include "Vector3.h"
#include <memory>

class GameBase;
class Camera;
class Object3d;

class Enemy {

	int HP = 2;
	bool isAlive = true;
	bool isStun_ = false;

	int stunTime;
	int stunTimeMax = 60 * 3;

	Vector3 direction_;
	Vector3 velocity_;
	Transform transform_;
	float maxSpeed_ = 0.1f;

	std::unique_ptr<Object3d> object_;
	std::unique_ptr<EnemyStun> enemyStun;

	Camera* camera_ = nullptr;

public:
	Enemy();
	~Enemy() = default;

	void Initialize(Camera* camera, Vector3 translate);
	void SetIsStun(bool isStun);
	void Update();
	void Draw();
	void Stun();
	void SetHPSubtract(int hp) { HP -= hp; }
	int GetHP() { return HP; }
	bool GetIsAlive() { return isAlive; }
	void SetCamera(Camera* camera) { camera_ = camera; }
	Vector3 GetPosition() { return transform_.translate; }
	Vector3 GetScale() { return transform_.scale; }
	void SetPosition() { transform_.translate; }
	void BulletCollision();
};
