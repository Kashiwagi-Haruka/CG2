#include "RotatingPlaygroundEquipment.h"
#include "Engine/Texture/Mesh/Animation/AnimationManager.h"
#include "Engine/Texture/Mesh/Model/ModelManager.h"
#include "Engine/Texture/Mesh/Object3d/Object3dCommon.h"
#include "Engine/base/DirectXCommon.h"
#include <cmath>

namespace {
constexpr const char* kGentlemanAnimationGroup = "Gentleman";
constexpr const char* kSpinAnimationGroup = "RotatingPlaygroundSpin";
} // namespace

RotatingPlaygroundEquipment::RotatingPlaygroundEquipment() = default;
RotatingPlaygroundEquipment::~RotatingPlaygroundEquipment() = default;

void RotatingPlaygroundEquipment::Initialize() {

	spinObj_ = std::make_unique<Object3d>();
	spinObj_->Initialize();
	ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/RotatingPlaygroundEquipment", "RotatingPlaygroundEquipment");
	ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/gentleman", "gentleman");
	AnimationManager::GetInstance()->LoadAnimationGroup(kSpinAnimationGroup, "Resources/TD3_3102/3d/RotatingPlaygroundEquipment", "RotatingPlaygroundEquipment");
	AnimationManager::GetInstance()->LoadAnimationGroup(kGentlemanAnimationGroup, "Resources/TD3_3102/3d/gentleman", "gentleman");
	spinObj_->SetModel("RotatingPlaygroundEquipment");
	if (Model* spinModel = ModelManager::GetInstance()->FindModel("RotatingPlaygroundEquipment")) {
		spinSkeleton_ = std::make_unique<Skeleton>(Skeleton().Create(spinModel->GetModelData().rootnode));
		spinSkinCluster_ = CreateSkinCluster(*spinSkeleton_, *spinModel);
		if (!spinSkinCluster_.mappedPalette.empty()) {
			spinObj_->SetSkinCluster(&spinSkinCluster_);
		}
	}
	spinTransform_.scale = {0.1f, 0.1f, 0.1f};
	spinTransform_.rotate = {0.0f, 0.0f, 0.0f};
	spinTransform_.translate = {0.0f, 0.0f, 0.0f};
	spinObj_->SetTransform(spinTransform_);
	if (const Animation::AnimationData* spinAnimation = AnimationManager::GetInstance()->FindAnimation(kSpinAnimationGroup, "spin")) {
		spinAnimation_ = spinAnimation;
		spinObj_->SetAnimation(spinAnimation_, spinLoopAnimation_);
	} else if (const Animation::AnimationData* spinAnimation = AnimationManager::GetInstance()->FindAnimation(kSpinAnimationGroup, "Spin")) {
		spinAnimation_ = spinAnimation;
		spinObj_->SetAnimation(spinAnimation_, spinLoopAnimation_);
	}

	constexpr size_t kGentlemanCount = 4;
	gentlemanObj_.resize(kGentlemanCount);
	gentlemanTransform_.resize(kGentlemanCount);
	gentlemanSkeleton_.resize(kGentlemanCount);
	gentlemanSkinCluster_.resize(kGentlemanCount);
	Model* gentlemanModel = ModelManager::GetInstance()->FindModel("gentleman");
	for (size_t i = 0; i < kGentlemanCount; ++i) {
		auto& gentleman = gentlemanObj_[i];
		gentleman = std::make_unique<Object3d>();
		gentleman->Initialize();
		gentleman->SetModel("gentleman");
		if (gentlemanModel) {
			gentlemanSkeleton_[i] = std::make_unique<Skeleton>(Skeleton().Create(gentlemanModel->GetModelData().rootnode));
			gentlemanSkinCluster_[i] = CreateSkinCluster(*gentlemanSkeleton_[i], *gentlemanModel);
			if (!gentlemanSkinCluster_[i].mappedPalette.empty()) {
				gentleman->SetSkinCluster(&gentlemanSkinCluster_[i]);
			}
		}

		auto& transform = gentlemanTransform_[i];
		transform.scale = {1.0f, 1.0f, 1.0f};
		transform.rotate = {0.0f, 0.0f, 0.0f};
		transform.translate = spinTransform_.translate;
		gentleman->SetTransform(transform);
		if (const Animation::AnimationData* roundAnimation = AnimationManager::GetInstance()->FindAnimation(kGentlemanAnimationGroup, "Round")) {
			gentleman->SetAnimation(roundAnimation, true);
		}
	}
}

void RotatingPlaygroundEquipment::Update() {
	if (spinSkeleton_ && spinAnimation_) {
		const float deltaTime = 1.0f / 60.0f; // Assuming a fixed frame rate of 60 FPS
		spinAnimationTime_ = Animation::AdvanceTime(*spinAnimation_, spinAnimationTime_, deltaTime, spinLoopAnimation_);
		spinSkeleton_->ApplyAnimation(*spinAnimation_, spinAnimationTime_);
		spinSkeleton_->Update();
		if (!spinSkinCluster_.mappedPalette.empty()) {
			UpdateSkinCluster(spinSkinCluster_, *spinSkeleton_);
		}
	}
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
	Object3dCommon::GetInstance()->DrawCommon();
	spinObj_->Draw();
	Object3dCommon::GetInstance()->DrawCommonSkinning();
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