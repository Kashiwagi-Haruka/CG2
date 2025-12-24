#include "PlayerSpecialAttack.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include <numbers>
#include "Function.h"
PlayerSpecialAttack::PlayerSpecialAttack() { 
	ModelManeger::GetInstance()->LoadModel("iceFlower"); 
}
void PlayerSpecialAttack::Initialize() {
	debugBox_ = std::make_unique<Object3d>();
	debugBox_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	debugBox_->SetCamera(camera_);
	debugBox_->SetModel("deBugBox");
	iceFlowers_ = std::make_unique<std::vector<Object3d>>();
	iceFlowers_->resize(6);
	iceFlowerTransforms_.resize(6);
	for (size_t i = 0; i < iceFlowers_->size(); i++) {
		(*iceFlowers_)[i] = Object3d();
		(*iceFlowers_)[i].Initialize(GameBase::GetInstance()->GetObject3dCommon());
		(*iceFlowers_)[i].SetCamera(camera_);
		(*iceFlowers_)[i].SetModel("iceFlower");
		iceFlowerTransforms_[i] = {
			.scale{1.0f, 1.0f, 1.0f},
			.rotate{0.0f, 0.0f, 0.0f},
			.translate{0.0f, 0.0f, 0.0f}
        };
		(*iceFlowers_)[i].SetTransform(iceFlowerTransforms_[i]);
	}
	rotateTimer = 0.0f;
	radiusTimer = 0.0f;
}
void PlayerSpecialAttack::Update(const Transform& playerTransform) {
	transform_ = playerTransform;
	rotateTimer += 0.05f;
	if (radiusTimer > 1.0f) {
		radiusTimer = 1.0f;
	}
	radiusTimer += 0.01f;
	radius = Function::Lerp(5.0f, 3.0f,radiusTimer);
	for (size_t i = 0; i < iceFlowers_->size(); i++) {
		float angle = (static_cast<float>(i) / iceFlowers_->size()) * 3.14159f * 2.0f;
		
		iceFlowerTransforms_[i].translate.x = transform_.translate.x + radius*cosf(angle+rotateTimer);
		iceFlowerTransforms_[i].translate.y = transform_.translate.y;
		iceFlowerTransforms_[i].translate.z = transform_.translate.z + radius * sinf(angle+rotateTimer);
		iceFlowerTransforms_[i].rotate.y = transform_.rotate.y+cosf(angle)*sinf(angle);
		(*iceFlowers_)[i].SetCamera(camera_);
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