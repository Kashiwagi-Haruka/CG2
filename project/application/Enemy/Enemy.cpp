#include "Enemy.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include "Object3d.h"
#include "Camera.h"
#include <algorithm>

Enemy::~Enemy() {

	delete object_; 
	
}

void Enemy::Initialize(GameBase* gameBase,Camera* camera) {
	isAlive = true;
	isHit = false;
	HP = 1;
	ModelManeger::GetInstance()->LoadModel("Enemy");
	object_ = new Object3d();
	object_->Initialize(gameBase->GetObject3dCommon());
	object_->SetModel("Enemy");
	camera_ = camera;
	transform_ = {
		.scale{1.0f,1.0f,1.0f},
		.rotate{0.0f,0.0f,0.0f},
		.translate{0.0f,0.0f,0.0f}
    };
	object_->SetTransform(transform_);
	object_->SetCamera(camera_);
	object_->Update();
}

void Enemy::Update(GameBase* gameBase) {
	// 敵の更新処理

	velocity_.x -= 0.01f;
	velocity_.x = std::clamp(velocity_.x, -maxSpeed_, maxSpeed_);
	transform_.translate += velocity_;

	object_->SetCamera(camera_);
	object_->SetTransform(transform_);
	object_->Update();
}

void Enemy::Draw(GameBase* gameBase) {
	// 敵の描画処理
	object_->Draw();
}
void Enemy::BulletCollision(){

}