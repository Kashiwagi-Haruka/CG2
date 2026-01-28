#pragma once
#include "EnemyAttack.h"
#include "EnemyStun.h"
#include "Primitive/Primitive.h"
#include "Transform.h"
#include "Vector3.h"
#include <memory>

class GameBase;
class Camera;
class Object3d;

class Enemy {

	int HP = 10;
	bool isAlive = true;
	bool isStun_ = false;

	int stunTime;
	int stunTimeMax = 60 * 3;

	float attackTimer_ = 0.0f;
	float attackCooldown_ = 0.6f;
	float attackRange_ = 1.5f;
	float attackHitSize_ = 1.2f;
	float damageInvincibleTimer_ = 0.0f;
	float damageInvincibleDuration_ = 0.3f;
	int lastSkillDamageId_ = -1;

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
	void Update(const Vector3& housePos, const Vector3& houseScale, const Vector3& playerPos, bool isPlayerAlive);
	void Draw();
	void Stun();
	void SetHPSubtract(int hp) {
		HP -= hp;
		if (HP <= 0) {
			isAlive = false;
		}
	}
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
	bool CanTakeDamage() const { return damageInvincibleTimer_ <= 0.0f; }
	void TriggerDamageInvincibility() { damageInvincibleTimer_ = damageInvincibleDuration_; }
	int GetLastSkillDamageId() const { return lastSkillDamageId_; }
	void SetLastSkillDamageId(int skillDamageId) { lastSkillDamageId_ = skillDamageId; }
	bool IsAttacking() const { return enemyAttack_ && enemyAttack_->IsAttacking(); }
	bool IsStunned() const { return isStun_; }
};