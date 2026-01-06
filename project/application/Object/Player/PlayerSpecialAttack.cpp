#include "PlayerSpecialAttack.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include "Object3dCommon.h"
#include <numbers>
PlayerSpecialAttack::PlayerSpecialAttack() { ModelManeger::GetInstance()->LoadModel("iceFlower"); }

void PlayerSpecialAttack::EnsureIceFlowerCount(int count) {
	if (count < 0) {
		count = 0;
	}
	if (!iceFlowers_) {
		iceFlowers_ = std::make_unique<std::vector<Object3d>>();
	}
	if (static_cast<int>(iceFlowers_->size()) == count) {
		return;
	}
	iceFlowers_->clear();
	iceFlowers_->resize(static_cast<size_t>(count));
	iceFlowerTransforms_.clear();
	iceFlowerTransforms_.resize(static_cast<size_t>(count));
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
}

void PlayerSpecialAttack::Initialize() {
	debugBox_ = std::make_unique<Object3d>();
	debugBox_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	debugBox_->SetCamera(camera_);
	debugBox_->SetModel("debugBox");
	iceFlowers_ = std::make_unique<std::vector<Object3d>>();
	iceFlowers_->clear();
	iceFlowerTransforms_.clear();
	isSpecialEnd_ = true;
	specialTime_ = 0;
}
void PlayerSpecialAttack::Update(const Transform& playerTransform) {
	if (isSpecialEnd_) {
		return;
	}
	transform_ = playerTransform;
	if (!iceFlowers_ || iceFlowers_->empty()) {
		isSpecialEnd_ = true;
		specialTime_ = 0;
		return;
	}
	for (size_t i = 0; i < iceFlowers_->size(); i++) {
		const float angle = (static_cast<float>(i) / iceFlowers_->size()) * std::numbers::pi_v<float> * 2.0f;

		iceFlowerTransforms_[i].translate.x = transform_.translate.x + radius * cosf(angle);
		iceFlowerTransforms_[i].translate.z = transform_.translate.z + radius * sinf(angle);
		iceFlowerTransforms_[i].translate.y -= fallSpeed_;
		iceFlowerTransforms_[i].rotate.y = transform_.rotate.y;
		(*iceFlowers_)[i].SetCamera(camera_);
		(*iceFlowers_)[i].SetTransform(iceFlowerTransforms_[i]);
		(*iceFlowers_)[i].Update();
	}
	if (specialTime_ > specialTimeMax_) {
		isSpecialEnd_ = true;
		specialTime_ = 0;
	} else {
		specialTime_++;
	}
	debugBox_->SetColor({1.0f, 0.0f, 1.0f, 0.5f});
	debugBox_->SetCamera(camera_);
	debugBox_->SetTransform(transform_);
	debugBox_->Update();
}
void PlayerSpecialAttack::StartAttack(const Transform& playerTransform, int iceCount) {
	transform_ = playerTransform;
	EnsureIceFlowerCount(iceCount);
	if (!iceFlowers_ || iceFlowers_->empty()) {
		isSpecialEnd_ = true;
		specialTime_ = 0;
		return;
	}
	isSpecialEnd_ = false;
	specialTime_ = 0;
	for (size_t i = 0; i < iceFlowers_->size(); i++) {
		const float angle = (static_cast<float>(i) / iceFlowers_->size()) * std::numbers::pi_v<float> * 2.0f;
		iceFlowerTransforms_[i].translate.x = transform_.translate.x + radius * cosf(angle);
		iceFlowerTransforms_[i].translate.z = transform_.translate.z + radius * sinf(angle);
		iceFlowerTransforms_[i].translate.y = transform_.translate.y + startHeight_;
		iceFlowerTransforms_[i].rotate.y = transform_.rotate.y;
		(*iceFlowers_)[i].SetCamera(camera_);
		(*iceFlowers_)[i].SetTransform(iceFlowerTransforms_[i]);
		(*iceFlowers_)[i].Update();
	}
}
void PlayerSpecialAttack::Draw() {
	GameBase::GetInstance()->GetObject3dCommon()->SetBlendMode(BlendMode::kBlendModeAdd);
	for (size_t i = 0; i < iceFlowers_->size(); i++) {
		(*iceFlowers_)[i].Draw();
	}
	GameBase::GetInstance()->GetObject3dCommon()->SetBlendMode(BlendMode::kBlendModeAlpha);
	debugBox_->Draw();
}