#include "Enemy.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include "Object3d.h"
#include "Camera.h"

Enemy::~Enemy() {

	delete object_; 
	
}

void Enemy::Initialize(GameBase* gameBase,Camera* camera) {
	ModelManeger::GetInstance()->LoadModel("Enemy");
	object_ = new Object3d();
	object_->Initialize(gameBase->GetObject3dCommon());
	object_->SetModel("Enemy");
	camera_ = camera;
	object_->SetCamera(camera_);
}

void Enemy::Update(GameBase* gameBase) {
	// 敵の更新処理
	object_->Update();
}

void Enemy::Draw(GameBase* gameBase) {
	// 敵の描画処理
	object_->Draw();
}