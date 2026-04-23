#include "Radicon.h"
#include "Model/ModelManager.h"
#include "Engine/math/Function.h"
#include "Engine/Texture/Mesh/Object3d/Object3dCommon.h"
#include "GameObject/KeyBindConfig.h"
void Radicon::Initialize() {
	obj_ = std::make_unique<Object3d>();
	ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/radicon", "Radicon");
	obj_->SetModel("Radicon");
	obj_->SetScale({ 0.01f, 0.01f, 0.01f });
	transform_ = {
		.scale = {0.01f, 0.01f, 0.01f},
		.rotate = {0.0f, 0.0f, 0.0f},
		.translate = {0.0f, 0.17f, 4.0f},
	};
	speed_ = 0.06f;
	obj_->SetTransform(transform_);
}
void Radicon::SetCamera(Camera* camera) {
	obj_->SetCamera(camera); }
void Radicon::Update(bool isOperationMode) {
	velocity_ = {0.0f, 0.0f, 0.0f};
	if (isOperationMode) {
		if (PlayerCommand::GetInstance()->MoveLeft()) {
			transform_.rotate.y -= 0.03f;
		}
		if (PlayerCommand::GetInstance()->MoveRight()) {
			transform_.rotate.y += 0.03f;
		}

		float throttle = 0.0f;
		if (PlayerCommand::GetInstance()->MoveForward()) {
			throttle += 1.0f;
		}
		if (PlayerCommand::GetInstance()->MoveBackward()) {
			throttle -= 1.0f;
		}
		velocity_ = Function::MakeForwardFromRotate(transform_.rotate) * (throttle * speed_);
		transform_.translate += velocity_;
	}

	transform_.translate.y = 0.17f;
	obj_->SetTransform(transform_);
	obj_->SetTranslate(transform_.translate);
	obj_->Update();
}
void Radicon::Draw() {
	Object3dCommon::GetInstance()->DrawCommonSkinning();
	obj_->Draw(); }
