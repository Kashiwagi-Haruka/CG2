#include "House.h"
#include "GameBase.h"
#include "ModelManeger.h"
House::House() {}

void House::Initialize(Camera* camera){
	ModelManeger::GetInstance()->LoadModel("house");
	object_ = new Object3d();
	object_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	object_->SetModel("house"); // ★ house のモデル名（必要なら変更）

	object_->SetCamera(camera);
	Vector3 scale = {8, 8, 8};
	position_ = {-12, 1.5f, 2.5f};
	object_->SetScale(scale);
	object_->SetTranslate(position_);
}

void House::Update(Camera* camera) {
	object_->SetCamera(camera);
	object_->SetTranslate(position_);
	object_->Update();
}
void House::Draw() {
	GameBase::GetInstance()->ModelCommonSet();
	object_->Draw();
}