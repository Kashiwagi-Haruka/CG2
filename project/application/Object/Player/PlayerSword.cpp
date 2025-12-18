#include "PlayerSword.h"
#include "GameBase.h"
#include "ModelManeger.h"
PlayerSword::PlayerSword(){ 

	ModelManeger::GetInstance()->LoadModel("playerSword");
	ModelManeger::GetInstance()->LoadModel("debugBox");
}

void PlayerSword::Initialize(){
	swordObject_ = std::make_unique<Object3d>();
	swordObject_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	swordObject_->SetCamera(camera);
	swordObject_->SetModel("playerSword");
#ifdef _DEBUG
	debugBox_ =  std::make_unique<Object3d>();
	debugBox_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	debugBox_->SetCamera(camera);
	debugBox_->SetModel("debugBox");
#endif // _DEBUG
}

void PlayerSword::Update(const Transform& playerTransform){
	Transform swordTransform = playerTransform;
	
	swordTransform.scale = {1.0f, 1.0f, 1.0f};
	swordTransform.translate.x += 1.0f;
	swordObject_->SetTransform(swordTransform);
	swordObject_->SetCamera(camera);
	swordObject_->Update();
#ifdef _DEBUG
	debugBox_->SetTransform(swordTransform);
	debugBox_->SetCamera(camera);
	debugBox_->Update();
#endif // _DEBUG
}

void PlayerSword::Draw(){
	swordObject_->Draw(); 
	#ifdef _DEBUG
	debugBox_->Draw();
	#endif // _DEBUG
}

	