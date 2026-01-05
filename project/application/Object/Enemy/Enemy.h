#pragma once
#include "EnemyStun.h"
#include "Transform.h"
#include "EnemyAttack.h"
#include "Vector3.h"
#include <memory>

class GameBase;
class Camera;
class Object3d;

class Enemy {

	int HP = 5;
	bool isAlive = true;
	bool isStun_ = false;

	int stunTime;
	int stunTimeMax = 60 * 3;

	float attackTimer_ = 0.0f;
	float attackCooldown_ = 1.0f;
	float attackRange_ = 1.5f;
	float attackHitSize_ = 1.2f;

	Vector3 direction_;
	Vector3 velocity_;
	Transform transform_;
	float maxSpeed_ = 0.1f;

	std::unique_ptr<Object3d> object_;
	std::unique_ptr<EnemyStun> enemyStun;
	std::unique_ptr<EnemyAttack> enemyAttack_;
	

	Camera* camera_ = nullptr;
	float playerChaseRange_ = 8.0f;

public:
	Enemy();
	~Enemy() = default;

	void Initialize(Camera* camera, Vector3 translate);
	void SetIsStun(bool isStun);
	void Update(const Vector3& housePos, const Vector3& playerPos, bool isPlayerAlive);
	void Draw();
	void Stun();
	void SetHPSubtract(int hp) { HP -= hp; }
	int GetHP() { return HP; }
	bool GetIsAlive() { return isAlive; }
	void SetCamera(Camera* camera) { camera_ = camera; }
	Vector3 GetPosition() { return transform_.translate; }
	Vector3 GetScale() { return transform_.scale; }
	void SetPosition(const Vector3& position) { transform_.translate = position; }
	float GetAttackRange() const { return attackRange_; }
	float GetAttackHitSize() const;
	Vector3 GetAttackPosition() const;
	bool IsAttackHitActive() const;
	bool IsAttackReady() const { return attackTimer_ >= attackCooldown_; }
	void ResetAttackTimer() { attackTimer_ = 0.0f; }
	void SetPosition() { transform_.translate; }
	void BulletCollision();
};
