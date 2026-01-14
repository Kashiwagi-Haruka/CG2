#include "SkyDome.h"
#include "GameBase.h"
#include "Model/ModelManager.h"
#include "Camera.h"

SkyDome::SkyDome() {
	ModelManager::GetInstance()->LoadModel("Resources/3d","skyDome");
	skyDomeObject_ = std::make_unique<Object3d>();
}


void SkyDome::Initialize(Camera* camera) {
	
	skyDomeObject_->Initialize();
	skyDomeObject_->SetModel("skyDome");
	camera_ = camera;
	skyDomeObject_->SetCamera(camera_);
	transform_.scale = {50.0f, 50.0f, 50.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {-75.0f, 2.50f, -75.0f};
	skyDomeObject_->SetTransform(transform_);
	skyDomeObject_->Update();
}
void SkyDome::Update() {
	skyDomeObject_->SetCamera(camera_);
	
	skyDomeObject_->Update();
}
void SkyDome::Draw() {
	skyDomeObject_->Draw(); }