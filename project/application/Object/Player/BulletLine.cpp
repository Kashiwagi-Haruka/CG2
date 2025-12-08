#include "BulletLine.h"
#include "Function.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include "Camera.h"
BulletLine::BulletLine() {
	ModelManeger::GetInstance()->LoadModel("Cube");
	lineObject_ = new Object3d();
	lineObject_->SetModel("Cube");
}

BulletLine::~BulletLine() { delete lineObject_; }

void BulletLine::Initialize() {
	lineObject_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	lineObject_->SetCamera(camera_);
}

void BulletLine::Update() {

	if (!camera_)
		return;

	lineObject_->SetCamera(camera_);

	// --- ① shotDir を正規化 ---
	Vector3 dir = shotDir_;
	if (Function::Length(dir) > 0.0001f) {
		dir = Function::Normalize(dir);
	} else {
		dir = {1, 0, 0}; // デフォルト右向き
	}

	// --- ② ラインのスケール（長さ = x方向に伸ばす） ---
	lineTransform_.scale = {lineLength_, 0.05f, 0.05f};

	// --- ③ dir を回転に変換（Z,X,Y の順で回す簡易版） ---
	float angleY = std::atan2(dir.x, dir.z); // Y回転（左右）
	float angleX = -std::asin(dir.y);        // X回転（上下）

	lineTransform_.rotate = {angleX, 0.0f, angleY};

	// --- ④ プレイヤーから前へ（lineLength_/2）だけ前に出した位置 ---
	lineTransform_.translate = playerPos_ + dir * (lineLength_ * 0.5f);

	// --- ⑤ Object3d に反映 ---
	lineObject_->SetScale(lineTransform_.scale);
	lineObject_->SetRotate(lineTransform_.rotate);
	lineObject_->SetTranslate(lineTransform_.translate);
	lineObject_->Update();
}

void BulletLine::Draw() { /*lineObject_->Draw();*/ }
