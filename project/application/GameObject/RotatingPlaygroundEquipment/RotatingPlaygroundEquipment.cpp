#include "RotatingPlaygroundEquipment.h"
#include "Engine/Texture/Mesh/Object3d/Object3dCommon.h"
#include "Engine/Texture/Mesh/Model/ModelManager.h"
void RotatingPlaygroundEquipment::Initialize() {
	spinObj_ = std::make_unique<Object3d>();
	ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/RotatingPlaygroundEquipment","RotatingPlaygroundEquipment");
	ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/gentleman", "gentleman");
	spinObj_->SetModel("RotatingPlaygroundEquipment");
	spinTransform_.scale = {0.01f, 0.01f, 0.01f};
	spinTransform_.rotate = {0.0f, 0.0f, 0.0f};
	spinTransform_.translate = {0.0f, 0.0f, 0.0f};
	spinObj_->SetTransform(spinTransform_);
	for (auto& gentleman : gentlemanObj_) {
		gentleman = std::make_unique<Object3d>();
		gentleman->Initialize();
		gentleman->SetModel("gentleman");
	}
}
void RotatingPlaygroundEquipment::Update() {
	spinTransform_.rotate.y += 1.0f;
	spinObj_->SetTransform(spinTransform_);
	spinObj_->Update();
}
void RotatingPlaygroundEquipment::Draw() {
	spinObj_->Draw(); 
}
void RotatingPlaygroundEquipment::SetCamera(Camera* camera) {
	camera_ = camera;
	spinObj_->SetCamera(camera);
	for (auto& gentleman : gentlemanObj_) {
		gentleman->SetCamera(camera);
	}
}