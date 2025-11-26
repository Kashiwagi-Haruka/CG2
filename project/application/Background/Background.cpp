#include "Background.h"
#include "GameBase.h"
#include "ModelManeger.h"
Background::Background() { 
	
	object_ = new Object3d(); 
	object2_ = new Object3d();
	object_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	object2_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	ModelManeger::GetInstance()->LoadModel("gameBG");

}
Background::~Background(){

	delete object2_;
	delete object_;
}
void Background::Initialize() { 
	
	translate = object_->GetTranslate();
	translate.x -= 16;
	translate.z = 1;
	initPosX = translate.x;
	translate2_ = translate;
	translate2_.x += 16;
	translate2_.z += 0.01f;
	object_->SetModel("gameBG");
	object_->SetCamera(camera_);
	object_->Update();
	object2_->SetModel("gameBG");
	object2_->SetCamera(camera_);
	object2_->Update();
}
void Background::Update(Vector3 playervelocity){ 
	object_->SetCamera(camera_);
	object2_->SetCamera(camera_);


	if (translate.x <= initPosX - 16) {
		translate.x = initPosX + 16;
	}
	if (translate2_.x <= initPosX - 16) {
		translate2_.x = initPosX + 16;
	}

	object_->SetTranslate(translate);
	object_->Update();
	object2_->SetTranslate(translate2_);
	object2_->Update();
}
void Background::Draw(){

	object_->Draw();
	object2_->Draw();

}
void Background::SetCamera(Camera* camera){

	camera_ = camera; }
void Background::SetPosition(Vector3 vector3) { object_->SetTranslate(vector3); }