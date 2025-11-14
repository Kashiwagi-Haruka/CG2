#include "Skydome.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include "Object3d.h"
#include "Camera.h"

Skydome::~Skydome() {
	delete skydomeObject_; 
}


void Skydome::Initialize(GameBase* gameBase,Camera* camera) {
	transform_ = {
	    .scale{2.0f, 2.0f, 2.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{0.0f, 0.0f, 0.0f}
    };
	ModelManeger::GetInstance()->LoadModel("skyDome");

	skydomeObject_ = new Object3d();

	skydomeObject_->Initialize(gameBase->GetObject3dCommon());

	skydomeObject_->SetModel("skyDome");

	camera_ = camera;
	skydomeObject_->SetCamera(camera_);
	skydomeObject_->SetScale(transform_.scale);
	skydomeObject_->Update();
}
void Skydome::Update() {
	
}
void Skydome::Draw(GameBase* gameBase) { 
	gameBase->ModelCommonSet();
	skydomeObject_->Draw();
}