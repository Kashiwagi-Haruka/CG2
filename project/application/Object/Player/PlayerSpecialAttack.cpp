#include "PlayerSpecialAttack.h"
#include "GameBase.h"
PlayerSpecialAttack::PlayerSpecialAttack() {}
void PlayerSpecialAttack::Initialize() {
	debugBox_ = std::make_unique<Object3d>();
	debugBox_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	debugBox_->SetCamera(camera_);
	debugBox_->SetModel("playerSpecialAttack");
	
}
void PlayerSpecialAttack::Update(const Transform& playerTransform) {
	transform_ = playerTransform;
	
	debugBox_->SetColor({1.0f, 0.0f, 1.0f, 0.5f});
	debugBox_->SetCamera(camera_);
	debugBox_->SetTransform(transform_);
	debugBox_->Update();
}
void PlayerSpecialAttack::Draw() {
	debugBox_->Draw(); 
}