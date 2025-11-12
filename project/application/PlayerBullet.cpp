#include "PlayerBullet.h"
#include "GameBase.h"
#include "Function.h"
#include "ModelManeger.h"
#include "Object3d.h"
#include "Camera.h"

PlayerBullet::~PlayerBullet() { 
	delete lineObject_;
	delete bulletObject_;
	
}

void PlayerBullet::Initialize(GameBase* gameBase,Camera* camera, Vector3 emitPos, Vector3 direction) {
	ModelManeger::GetInstance()->LoadModel("playerBullet");
	ModelManeger::GetInstance()->LoadModel("Cube");
	bulletObject_ = new Object3d();
	lineObject_ = new Object3d();
	bulletObject_->Initialize(gameBase->GetObject3dCommon());
	lineObject_->Initialize(gameBase->GetObject3dCommon());
	bulletObject_->SetModel("playerBullet");
	lineObject_->SetModel("Cube");
	camera_ = camera;
	bulletObject_->SetCamera(camera_);
	lineObject_->SetCamera(camera_);
	transform_.translate = emitPos;
	bulletObject_->SetTranslate(transform_.translate);
	direction_ = direction;
	bulletObject_->Update();
	lineObject_->Update();
}

void PlayerBullet::Update(GameBase* gameBase) { 
	
	
}
void PlayerBullet::Fire(){



	transform_.translate += direction_ * 0.1f;
	bulletObject_->SetTranslate(transform_.translate);

	bulletObject_->Update();
}



void PlayerBullet::Draw(GameBase* gameBase) {

	bulletObject_->Draw();
	
}