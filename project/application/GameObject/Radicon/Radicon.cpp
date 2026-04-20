#include "Radicon.h"
#include "Model/ModelManager.h"
void Radicon::Initialize() {
	obj_ = std::make_unique<Object3d>();
	ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/radicon", "Radicon");
	obj_->SetModel("Radicon");
	obj_->SetScale({ 0.01f, 0.01f, 0.01f });
	speed_ = 0.1f;
}
void Radicon::SetCamera(Camera* camera) {
	obj_->SetCamera(camera); }
void Radicon::Update() {
	velocity_.x = speed_;
	obj_->SetTranslate({
		obj_->GetTransform().translate.x + velocity_.x,
		obj_->GetTransform().translate.y,
		obj_->GetTransform().translate.z
	});
	obj_->Update();
}
void Radicon::Draw() {
	obj_->Draw(); }