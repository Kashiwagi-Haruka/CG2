#include "CharacterDisplay.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include <numbers>

void CharacterDisplay::Initialize() {
	camera_ = std::make_unique<Camera>();
	camera_->SetTransform(cameraTransform_);
	camera_->Update();


	characterObject_ = std::make_unique<Object3d>();
	characterObject_->Initialize();
	characterObject_->SetCamera(camera_.get());
	characterObject_->SetModel("sizuku");
	characterObject_->SetTransform(characterTransform_);
	characterObject_->SetShininess(80.0f);
	characterObject_->SetEnvironmentCoefficient(0.35f);
	characterObject_->Update();
}

void CharacterDisplay::Update() {
	if (!isActive_ || !characterObject_ || !camera_) {
		return;
	}
	camera_->SetTransform(cameraTransform_);
	camera_->Update();
	characterObject_->SetCamera(camera_.get());
	characterObject_->SetTransform(characterTransform_);
	characterObject_->Update();
}

void CharacterDisplay::Draw() {
	if (!isActive_ || !characterObject_) {
		return;
	}
	Object3dCommon::GetInstance()->DrawCommonSkinningToon();
	characterObject_->Draw();
}