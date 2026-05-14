#include "RotatingPlaygroundEquipment.h"
#include "Engine/Texture/Mesh/Model/ModelManager.h"
#include "Engine/Texture/Mesh/Object3d/Object3dCommon.h"
#include <cmath>

RotatingPlaygroundEquipment::RotatingPlaygroundEquipment() = default;
RotatingPlaygroundEquipment::~RotatingPlaygroundEquipment() = default;

void RotatingPlaygroundEquipment::Initialize() {
	spinObj_ = std::make_unique<Object3d>();
	spinObj_->Initialize();
	ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/RotatingPlaygroundEquipment", "RotatingPlaygroundEquipment");
	ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/gentleman", "gentleman");
	spinObj_->SetModel("RotatingPlaygroundEquipment");
	spinTransform_.scale = {0.01f, 0.01f, 0.01f};
	spinTransform_.rotate = {0.0f, 0.0f, 0.0f};
	spinTransform_.translate = {0.0f, 0.0f, 0.0f};
	spinObj_->SetTransform(spinTransform_);

	constexpr size_t kGentlemanCount = 4;
	gentlemanObj_.resize(kGentlemanCount);
	gentlemanTransform_.resize(kGentlemanCount);
	for (size_t i = 0; i < kGentlemanCount; ++i) {
		auto& gentleman = gentlemanObj_[i];
		gentleman = std::make_unique<Object3d>();
		gentleman->Initialize();
		gentleman->SetModel("gentleman");

		auto& transform = gentlemanTransform_[i];
		transform.scale = {0.01f, 0.01f, 0.01f};
		transform.rotate = {0.0f, 0.0f, 0.0f};
		transform.translate = spinTransform_.translate;
		gentleman->SetTransform(transform);
	}
}

void RotatingPlaygroundEquipment::Update() {
	spinTransform_.rotate.y += 0.01f;
	spinObj_->SetTransform(spinTransform_);
	spinObj_->Update();

	gentlemanOrbitAngle_ += gentlemanOrbitSpeed_;
	constexpr float kPi = 3.1415926535f;
	const float angleStep = (2.0f * kPi) / static_cast<float>(gentlemanObj_.size());
	for (size_t i = 0; i < gentlemanObj_.size(); ++i) {
		const float angle = gentlemanOrbitAngle_ + (angleStep * static_cast<float>(i));
		auto& transform = gentlemanTransform_[i];
		transform.translate = {
		    spinTransform_.translate.x + std::cosf(angle) * gentlemanOrbitRadius_, spinTransform_.translate.y, spinTransform_.translate.z + std::sinf(angle) * gentlemanOrbitRadius_};
		transform.rotate.y = angle + kPi;
		gentlemanObj_[i]->SetTransform(transform);
		gentlemanObj_[i]->Update();
	}
}

void RotatingPlaygroundEquipment::Draw() {
	Object3dCommon::GetInstance()->DrawCommonSkinning();
	spinObj_->Draw();
	for (auto& gentleman : gentlemanObj_) {
		gentleman->Draw();
	}
}

void RotatingPlaygroundEquipment::SetCamera(Camera* camera) {
	camera_ = camera;
	spinObj_->SetCamera(camera);
	for (auto& gentleman : gentlemanObj_) {
		gentleman->SetCamera(camera);
	}
}