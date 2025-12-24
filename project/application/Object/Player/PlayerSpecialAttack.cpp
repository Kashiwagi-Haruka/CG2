#include "PlayerSpecialAttack.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include <numbers>
PlayerSpecialAttack::PlayerSpecialAttack() { 
	ModelManeger::GetInstance()->LoadModel("iceFlower"); 
}
void PlayerSpecialAttack::Initialize() {
	debugBox_ = std::make_unique<Object3d>();
	debugBox_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	debugBox_->SetCamera(camera_);
	debugBox_->SetModel("playerSpecialAttack");
	iceFlowers_ = std::make_unique<std::vector<Object3d>>();
	iceFlowers_->resize(6);
	iceFlowerTransforms_.resize(6);
	for (size_t i = 0; i < iceFlowers_->size(); i++) {
		(*iceFlowers_)[i] = Object3d();
		(*iceFlowers_)[i].Initialize(GameBase::GetInstance()->GetObject3dCommon());
		(*iceFlowers_)[i].SetCamera(camera_);
		(*iceFlowers_)[i].SetModel("playerSpecialAttack");
		iceFlowerTransforms_[i] = {
			.scale{1.0f, 1.0f, 1.0f},
			.rotate{std::numbers::pi_v<float>/2.0f, 0.0f, 0.0f},
			.translate{0.0f, 0.0f, 0.0f}
        };
		(*iceFlowers_)[i].SetTransform(iceFlowerTransforms_[i]);
	}
	
}
void PlayerSpecialAttack::Update(const Transform& playerTransform) {
	transform_ = playerTransform;
	
	for (size_t i = 0; i < iceFlowers_->size(); i++) {
		float angle = (static_cast<float>(i) / iceFlowers_->size()) * 3.14159f * 2.0f;
		float radius = 3.0f; 
		iceFlowerTransforms_[i].translate.x = transform_.translate.x + 1.0f;
		iceFlowerTransforms_[i].translate.y = transform_.translate.y;
		iceFlowerTransforms_[i].translate.z = transform_.translate.z/* + radius * sinf(angle)*/;
		(*iceFlowers_)[i].SetTransform(iceFlowerTransforms_[i]);
		(*iceFlowers_)[i].Update();
	}
	debugBox_->SetColor({1.0f, 0.0f, 1.0f, 0.5f});
	debugBox_->SetCamera(camera_);
	debugBox_->SetTransform(transform_);
	debugBox_->Update();
}
void PlayerSpecialAttack::Draw() {
	for (size_t i = 0; i < iceFlowers_->size(); i++) {
		(*iceFlowers_)[i].Draw();
	}
	debugBox_->Draw(); 
}