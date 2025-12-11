#include "Enemy.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include "Object3d.h"
#include "Camera.h"
#include <algorithm>

Enemy::Enemy() {
	ModelManeger::GetInstance()->LoadModel("Enemy");
	object_ = std::make_unique<Object3d>();
	enemyStun = std::make_unique<EnemyStun>();
}


void Enemy::Initialize(Camera* camera,Vector3 translates) {
	isAlive = true;
	isStun_ = false;
	HP = 3;
	
	object_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
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
}

void Enemy::Update() {
	// 敵の更新処理
	if (!isStun_) {
	velocity_.x -= 0.01f;
	velocity_.x = std::clamp(velocity_.x, -maxSpeed_, maxSpeed_);
	
	}

	if (isStun_) {
		velocity_.x = 0;
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
	if (isStun_) {
		enemyStun->Draw();
	}

}
void Enemy::BulletCollision(){

}
void Enemy::SetIsStun(bool IsStun){ isStun_ = IsStun; }