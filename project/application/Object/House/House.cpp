#include "House.h"
#include "GameBase.h"
#include "ModelManeger.h"
House::House() {}

void House::Initialize(Camera* camera) {

	ModelManeger::GetInstance()->LoadModel("house");
	object_ = std::make_unique<Object3d>();

	object_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	object_->SetModel("house");

	object_->SetCamera(camera);
	object_->SetScale({4, 4, 4});
	position_ = {-62, 1.5f, -47.5f};
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
