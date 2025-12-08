#include "PlayerBullet.h"
#include "GameBase.h"
#include "Function.h"
#include "ModelManeger.h"
#include "Object3d.h"
#include "Camera.h"

PlayerBullet::PlayerBullet() {
	
	

	


}
PlayerBullet::~PlayerBullet() { 

	delete object_;
	
}

void PlayerBullet::Initialize(Camera* camera) {
	isCharge_ = false;
	isAirBullet = false;
	object_ = new Object3d();
	object_->SetModel("playerBullet");
	object_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	


	camera_ = camera;
	object_->SetCamera(camera_);

}

void PlayerBullet::Update(Camera* camera) { 
	// 発射：velocity_ に沿って前進
	transform_.translate += velocity_;

	object_->SetTranslate(transform_.translate);

	object_->SetCamera(camera);
	object_->Update();



}



void PlayerBullet::Charge(Vector3 playerPos, Vector3 direction) {
	isCharge_ = true;
	// direction はすでに正規化された入力方向
	float distance = 2.0f; // プレイヤーの前に出す距離

	transform_.translate = playerPos + direction * distance;

	// 弾の更新
	object_->SetTranslate(transform_.translate);
	object_->Update();

	// ----- 線（プレイヤー～弾） -----
	
}

void PlayerBullet::Fire() {
	isCharge_ = false;
	// 発射時の位置を明確にする
	object_->SetTranslate(transform_.translate);
	object_->Update();
}



void PlayerBullet::Draw() {

	GameBase::GetInstance()->ModelCommonSet();
	object_->Draw();
	
	
}

void PlayerBullet::SetVelocity(Vector3 verocity){ velocity_ = verocity; }