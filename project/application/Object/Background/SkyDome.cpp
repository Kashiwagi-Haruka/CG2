#include "SkyDome.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include "Camera.h"

SkyDome::SkyDome() {
	ModelManeger::GetInstance()->LoadModel("skyDome");
	skyDomeObject_ = std::make_unique<Object3d>();
}


void SkyDome::Initialize(Camera* camera) {
	
	skyDomeObject_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	skyDomeObject_->SetModel("skyDome");
	camera_ = camera;
	skyDomeObject_->SetCamera(camera_);
	transform_.scale = {25.0f, 25.0f, 25.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {-50.0f, 2.50f, -50.0f};
	skyDomeObject_->SetTransform(transform_);
	skyDomeObject_->Update();
}
void SkyDome::Update() {
	skyDomeObject_->SetCamera(camera_);
	
	skyDomeObject_->Update();
}
void SkyDome::Draw() {
	skyDomeObject_->Draw(); }