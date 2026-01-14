#include "Enemy.h"
#include "GameBase.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3d.h"
#include "Camera.h"
#include <algorithm>

Enemy::Enemy() {
	ModelManager::GetInstance()->LoadModel("Enemy");
	object_ = std::make_unique<Object3d>();
	enemyStun = std::make_unique<EnemyStun>();
	
}


void Enemy::Initialize(Camera* camera,Vector3 translates) {
	isAlive = true;
	isStun_ = false;
	HP = 3;
	
	object_->Initialize();
	object_->SetModel("Enemy");
	camera_ = camera;
	transform_ = {
		.scale{1.0f,1.0f,1.0f},
		.rotate{0.0f,0.0f,0.0f},
		.translate=translates
    };
	object_->SetTransform(transform_);
	object_->SetCamera(camera_);
	object_->Update();

	enemyStun->Initialize();
	enemyAttack_ = std::make_unique<EnemyAttack>();
	enemyAttack_->Initialize(camera_);
}

void Enemy::Update(const Vector3& housePos, const Vector3& playerPos, bool isPlayerAlive) {
	attackTimer_ += 1.0f / 60.0f;
	// 敵の更新処理
	if (!isStun_) {
		Vector3 target = housePos;
		if (isPlayerAlive) {
			Vector3 toPlayer = playerPos - transform_.translate;
			if (LengthSquared(toPlayer) <= playerChaseRange_ * playerChaseRange_) {
				target = playerPos;
			}
		}

		Vector3 toTarget = target - transform_.translate;
		toTarget.y = 0.0f;
		if (LengthSquared(toTarget) > 0.0001f) {
			Vector3 direction = Function::Normalize(toTarget);
			velocity_ = direction * maxSpeed_;
		} else {
			velocity_ = {0.0f, 0.0f, 0.0f};
		}
	
	}

	if (isStun_) {
		velocity_ = {0.0f, 0.0f, 0.0f};
		stunTime++;
		enemyStun->SetCamera(camera_);
		enemyStun->SetTranslate(transform_.translate);
		enemyStun->Update();
		if (stunTime >= stunTimeMax) {
			isStun_ = false;
		}
	}
	transform_.translate += velocity_;
	object_->SetCamera(camera_);
	object_->SetTransform(transform_);
	object_->Update();
	// 攻撃タイマー更新
	attackTimer_ += 1.0f / 60.0f;

	// 攻撃開始条件
	if (!isStun_ && IsAttackReady()) {
		enemyAttack_->Start(transform_);
		ResetAttackTimer();
	}

	enemyAttack_->Update();

	if (HP <= 0) {
		isAlive = false;
	}
}
void Enemy::Stun() {
	isStun_ = true;
	if (HP <= 0) {
		isAlive = false;
	}
}

void Enemy::Draw() {
	// 敵の描画処理
	object_->Draw();

	if (enemyAttack_) {
		enemyAttack_->Draw();
	}

	if (isStun_) {
		enemyStun->Draw();
	}


}
void Enemy::BulletCollision(){

}

void Enemy::SetIsStun(bool IsStun) { isStun_ = IsStun; }

float Enemy::GetAttackHitSize() const {
	if (enemyAttack_) {
		return enemyAttack_->GetHitSize();
	}
	return attackHitSize_;
}

Vector3 Enemy::GetAttackPosition() const {
	if (enemyAttack_) {
		return enemyAttack_->GetPosition();
	}
	return transform_.translate;
}

bool Enemy::IsAttackHitActive() const { return enemyAttack_ && enemyAttack_->IsHitActive(); }