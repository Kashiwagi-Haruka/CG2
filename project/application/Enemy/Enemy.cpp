#include "Enemy.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include "Object3d.h"
#include "Camera.h"
#include <algorithm>

Enemy::Enemy(){
	ModelManeger::GetInstance()->LoadModel("Enemy");
	object_ = new Object3d();
}

Enemy::~Enemy() {

	delete object_; 
	
}

void Enemy::Initialize(Camera* camera,Vector3 translates) {
	isAlive = true;
	isHit = false;
	HP = 2;
	
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
}

void Enemy::Update() {
	// 敵の更新処理

	velocity_.x -= 0.01f;
	velocity_.x = std::clamp(velocity_.x, -maxSpeed_, maxSpeed_);
	transform_.translate += velocity_;

	object_->SetCamera(camera_);
	object_->SetTransform(transform_);
	object_->Update();
	if (HP <= 0) {
		isAlive = false;
	}
}
void Enemy::Stun() {
	// ノックバック


	HP -= 1; // ダメージ
	if (HP <= 0) {
		isAlive = false;
	}
}

void Enemy::Draw() {
	// 敵の描画処理
	object_->Draw();
}
void Enemy::BulletCollision(){

}