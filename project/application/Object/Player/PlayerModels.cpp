#include "PlayerModels.h"
#include "Function.h"
#include "GameBase.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "imgui.h"
#include <cmath>
#include <numbers>

PlayerModels::PlayerModels() {

};
PlayerModels::~PlayerModels() {};

void PlayerModels::Initialize() {

	Sizuku_ = std::make_unique<Object3d>();

	Sizuku_->SetModel("sizuku");

	Sizuku_->Initialize();

	sizukuAnimationClips_ = Animation::LoadAnimationClips("Resources/3d", "sizuku");
	if (!sizukuAnimationClips_.empty()) {
		sizukuAnimationIndex_ = 0;
		for (size_t i = 0; i < sizukuAnimationClips_.size(); ++i) {
			if (sizukuAnimationClips_[i].name == "Idle") {
				sizukuAnimationIndex_ = i;
				break;
			}
		}
		Sizuku_->SetAnimation(&sizukuAnimationClips_[sizukuAnimationIndex_], true);
	}

	if (Model* sizukuModel = ModelManager::GetInstance()->FindModel("sizuku")) {
		sizukuSkeleton_ = std::make_unique<Skeleton>(Skeleton().Create(sizukuModel->GetModelData().rootnode));
		sizukuSkinCluster_ = CreateSkinCluster(ModelManager::GetInstance()->GetModelCommon(), *sizukuSkeleton_, *sizukuModel);
		if (!sizukuSkinCluster_.mappedPalette.empty()) {
			Sizuku_->SetSkinCluster(&sizukuSkinCluster_);
		}
	}
}

void PlayerModels::Update() {

	static float t = 0.0f;
	t += 0.1f;
	switch (state_) {
	case PlayerModels::idle:

		break;
	case PlayerModels::walk:

		break;
	case PlayerModels::attack1:

		break;
	case PlayerModels::attack2:

		break;
	case PlayerModels::attack3:

		break;
	case PlayerModels::attack4:

		break;
	case PlayerModels::fallingAttack:

		break;
	case PlayerModels::skillAttack:

		break;
	case PlayerModels::damage:

		break;
	default:
		break;
	}

	if (sizukuSkeleton_ && !sizukuAnimationClips_.empty()) {
		const auto& currentAnimation = sizukuAnimationClips_[sizukuAnimationIndex_];
		const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
		sizukuSkeleton_->UpdateAnimation(currentAnimation, sizukuAnimationTime_, deltaTime);
		if (!sizukuSkinCluster_.mappedPalette.empty()) {
			UpdateSkinCluster(sizukuSkinCluster_, *sizukuSkeleton_);
		}
	}

#ifdef USE_IMGUI
	if (!ImGui::Begin("Player Parts Adjust")) {
		ImGui::End();
		return;
	}

	if (ImGui::CollapsingHeader("Player Root")) {
		ImGui::DragFloat3("Player Pos", &player_.translate.x, 0.01f);
		ImGui::DragFloat3("Player Rot", &player_.rotate.x, 0.01f);
		ImGui::DragFloat3("Player Scale", &player_.scale.x, 0.01f);
	}

	ImGui::End();
#endif
}

void PlayerModels::Draw() {

	Matrix4x4 playerWorld = Function::MakeAffineMatrix(player_.scale, player_.rotate, player_.translate);

	Sizuku_->SetWorldMatrix(playerWorld);
	Sizuku_->SetCamera(camera_);
	Sizuku_->Update();
	Object3dCommon::GetInstance()->DrawCommonSkinning();
	Sizuku_->Draw();

#ifdef _DEBUG
	if (sizukuSkeleton_) {
		sizukuSkeleton_->SetObjectMatrix(playerWorld);
		Object3dCommon::GetInstance()->DrawCommonWireframeNoDepth();
		sizukuSkeleton_->DrawBones(camera_, {0.2f, 0.9f, 1.0f, 1.0f}, {0.1f, 0.5f, 0.9f, 1.0f});
	}
#endif
}