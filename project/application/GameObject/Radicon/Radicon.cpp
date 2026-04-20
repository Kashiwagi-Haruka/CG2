#include "Radicon.h"
#include "Model/ModelManager.h"
#include "Engine/math/Function.h"
#include "Engine/Texture/Mesh/Object3d/Object3dCommon.h"
void Radicon::Initialize() {
	obj_ = std::make_unique<Object3d>();
	ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/radicon", "Radicon");
	obj_->SetModel("Radicon");
	obj_->SetScale({ 0.01f, 0.01f, 0.01f });
	speed_ = 0.1f;
	transform_ = obj_->GetTransform();
}
void Radicon::SetCamera(Camera* camera) {
	obj_->SetCamera(camera); }
void Radicon::Update() {
	velocity_.x += speed_ * Function::MakeForwardFromRotate(transform_.rotate).x;
	velocity_.z += speed_ * Function::MakeForwardFromRotate(transform_.rotate).z;
	transform_.translate.x += velocity_.x;
	transform_.translate.z += velocity_.z;
	obj_->SetTranslate(transform_.translate);
	obj_->Update();
}
void Radicon::Draw() {
	Object3dCommon::GetInstance()->DrawCommonSkinning();
	obj_->Draw(); }