#include "PlayerBullet.h"
#include "GameBase.h"
#include "Function.h"
#include "ModelManeger.h"
#include "Object3d.h"
#include "Camera.h"

PlayerBullet::~PlayerBullet() { 
	delete object_;
	delete camera_;
}

void PlayerBullet::Initialize(GameBase* gameBase,Camera* camera, Vector3 emitPos, Vector3 direction) {
	ModelManeger::GetInstance()->LoadModel("playerBullet");
	object_ = new Object3d();
	object_->Initialize(gameBase->GetObject3dCommon());
	object_->SetModel("playerBullet");
	camera_ = camera;
	object_->SetCamera(camera_);
	transform_.translate = emitPos;
	object_->SetTranslate(transform_.translate);
	direction_ = direction;
	object_->Update();
}

void PlayerBullet::Update(GameBase* gameBase) { 
	
	transform_.translate += direction_ * 0.1f;
	object_->SetTranslate(transform_.translate);
	
	object_->Update();
}

void PlayerBullet::Draw(GameBase* gameBase) {

	object_->Draw();

}