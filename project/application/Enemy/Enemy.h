#pragma once
#include "Transform.h"
#include "Vector3.h"
#include "EnemyStun.h"
class GameBase;
class Camera;
class Object3d;

class Enemy {

	int HP = 2;
	bool isAlive = true; // 生きているか
	bool isStun_ = false;//踏まれたか

	int stunTime;
	int stunTimeMax=60*3;

	Vector3 direction_;
	Vector3 velocity_;
	Transform transform_;

	float maxSpeed_ = 0.1f;


	Object3d* object_ = nullptr;
	Camera* camera_ = nullptr;
	EnemyStun* enemyStun = nullptr;

public:
	Enemy();
	~Enemy();
	void Initialize(Camera* camera,Vector3 translate);
	void SetIsStun(bool isStun);
	void Update();
	void Draw();
	void Stun(); // ★追加
	void SetHP(int hp) { HP = hp; }

	bool GetIsAlive() { return isAlive; }
	void SetCamera(Camera* camera) { camera_ = camera;}
	Vector3 GetPosition() { return transform_.translate; }
	void SetPosition() { transform_.translate; }
	void BulletCollision();
};
