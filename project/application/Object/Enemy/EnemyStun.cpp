#include "EnemyStun.h"
#include "ModelManeger.h"
#include "GameBase.h"

EnemyStun::EnemyStun() { 
	ModelManeger::GetInstance()->LoadModel("EnemyStun");
	object_ = new Object3d(); 
	object_->SetModel("EnemyStun");
}

EnemyStun::~EnemyStun(){ 
	delete object_;

}
void EnemyStun::SetCamera(Camera* camera) { camera_ = camera; }
void EnemyStun::SetTranslate(Vector3 translate) { transform_.translate = translate; };
void EnemyStun::Initialize() { 
	
	object_->SetCamera(camera_);
	object_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	object_->SetTransform(transform_);
	object_->Update();

}

void EnemyStun::Update() { 
	object_->Update(); 
}

void EnemyStun::Draw() { object_->Draw(); }