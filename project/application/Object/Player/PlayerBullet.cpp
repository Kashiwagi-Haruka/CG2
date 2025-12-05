#include "PlayerBullet.h"
#include "GameBase.h"
#include "Function.h"
#include "ModelManeger.h"
#include "Object3d.h"
#include "Camera.h"

PlayerBullet::PlayerBullet() {
	ModelManeger::GetInstance()->LoadModel("playerBullet");
	ModelManeger::GetInstance()->LoadModel("Cube");
	bulletObject_ = new Object3d();
	lineObject_ = new Object3d();


}
PlayerBullet::~PlayerBullet() { 
	delete lineObject_;
	delete bulletObject_;
	
}

void PlayerBullet::Initialize(Camera* camera) {
	isCharge_ = false;
	bulletObject_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	lineObject_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	bulletObject_->SetModel("playerBullet");
	lineObject_->SetModel("Cube");
	camera_ = camera;
	bulletObject_->SetCamera(camera_);
	lineObject_->SetCamera(camera_);
}

void PlayerBullet::Update(Camera* camera) { 
	// 発射：velocity_ に沿って前進
	bulletTransform_.translate += velocity_;

	bulletObject_->SetTranslate(bulletTransform_.translate);
	bulletObject_->Update();
	bulletObject_->SetCamera(camera);
	lineObject_->SetCamera(camera);



}

void PlayerBullet::AirBullet(){




}

void PlayerBullet::Charge(Vector3 playerPos, Vector3 direction) {
	isCharge_ = true;
	// direction はすでに正規化された入力方向
	float distance = 2.0f; // プレイヤーの前に出す距離

	bulletTransform_.translate = playerPos + direction * distance;

	// 弾の更新
	bulletObject_->SetTranslate(bulletTransform_.translate);
	bulletObject_->Update();

	// ----- 線（プレイヤー～弾） -----
	Vector3 diff = bulletTransform_.translate - playerPos;
	float len = Function::Length(diff);

	// ラインの中心位置
	lineTransform_.translate = playerPos + diff * 0.5f;

	// スケール（長さ方向だけ伸ばす）
	lineTransform_.scale = {len, 0.1f, 0.1f};

	// 方向を向ける
	Vector3 axis = {1, 0, 0}; // Cube は X 軸方向が長い前提
	lineTransform_.rotate = Function::DirectionToRotation(Function::Normalize(diff), axis);

	lineObject_->SetScale(lineTransform_.scale);
	lineObject_->SetRotate(lineTransform_.rotate);
	lineObject_->SetTranslate(lineTransform_.translate);
	lineObject_->Update();
}

void PlayerBullet::Fire() {
	isCharge_ = false;

	// 線は発射後は消すなら描画しない or scale 0 に
	lineTransform_.scale = {0, 0, 0};
	lineObject_->SetScale(lineTransform_.scale);
}


void PlayerBullet::Draw() {

	bulletObject_->Draw();
	if (isCharge_)
	lineObject_->Draw();
}

void PlayerBullet::SetVelocity(Vector3 verocity){ velocity_ = verocity; }