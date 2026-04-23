#include "Enemy.h"
#include "Camera.h"
#include "Mesh/Object3d/Object3dCommon.h"
#include "Engine/Texture/Mesh/Model/ModelManager.h"
Enemy::Enemy() { obj_ = std::make_unique<Object3d>(); }
void Enemy::Initialize() { 
	obj_->Initialize();
	obj_->SetColor({0.3f, 0.3f, 0.3f, 1.0f});
	// モデルの読み込み
	ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/gentleman", "gentleman");
	obj_->SetModel("gentleman");
}
void Enemy::SetCamera(Camera* camera) {
	obj_->SetCamera(camera); }
void Enemy::Update() {
	obj_->SetTransform(transform_);
	obj_->Update();
}
void Enemy::Draw() {
	Object3dCommon::GetInstance()->DrawCommonSkinning();
	obj_->Draw(); }