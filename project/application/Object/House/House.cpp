#include "House.h"
#include "GameBase.h"
#include "ModelManeger.h"
House::House() {}

void House::Initialize(Camera* camera) {

	ModelManeger::GetInstance()->LoadModel("house");
	ModelManeger::GetInstance()->LoadModel("HPBar");
	ModelManeger::GetInstance()->LoadModel("HPFlame");
	object_ = std::make_unique<Object3d>();

	object_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	object_->SetModel("house");

	object_->SetCamera(camera);
	object_->SetScale({1, 1, 1});
	position_ = {-62, 1.5f, -47.5f};
	object_->SetTranslate(position_);

	hpbar_ = std::make_unique<Object3d>();
	hpbar_->SetModel("HPBar");
	hpbar_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	hpbar_->SetCamera(camera);

	hpflame_ = std::make_unique<Object3d>();
	hpflame_->SetModel("HPFlame");
	hpflame_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	hpflame_->SetCamera(camera);

}

void House::Update(Camera* camera) {
	object_->SetCamera(camera);
	object_->SetTranslate(position_);
	object_->Update();

	hpflame_->SetCamera(camera);
	hpflame_->Update();

	hpbar_->SetCamera(camera);
	hpbar_->Update();
}

void House::Draw() {
	GameBase::GetInstance()->ModelCommonSet();
	object_->Draw();
	hpflame_->Draw();
	hpbar_->Draw();
}
