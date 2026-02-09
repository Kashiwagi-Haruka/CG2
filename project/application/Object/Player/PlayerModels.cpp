#include "PlayerModels.h"
#include "Function.h"
#include "GameBase.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "imgui.h"
#include <cmath>
#include <numbers>

PlayerModels::PlayerModels() : state_(StateM::idle) {};
PlayerModels::~PlayerModels() {};

void PlayerModels::Initialize() {

	Sizuku_ = std::make_unique<Object3d>();

	Sizuku_->SetModel("sizuku");

	Sizuku_->Initialize();

	AnimationManager::GetInstance()->LoadAnimationGroup(animationGroupName_, "Resources/3d", "sizuku");
	AnimationManager::GetInstance()->ResetPlayback(animationGroupName_, "Idle", true);
	if (const Animation::AnimationData* idleAnimation = AnimationManager::GetInstance()->FindAnimation(animationGroupName_, "Idle")) {
		Sizuku_->SetAnimation(idleAnimation, true);
	}

	if (Model* sizukuModel = ModelManager::GetInstance()->FindModel("sizuku")) {
		sizukuSkeleton_ = std::make_unique<Skeleton>(Skeleton().Create(sizukuModel->GetModelData().rootnode));
		sizukuSkinCluster_ = CreateSkinCluster(*sizukuSkeleton_, *sizukuModel);
		if (!sizukuSkinCluster_.mappedPalette.empty()) {
			Sizuku_->SetSkinCluster(&sizukuSkinCluster_);
		}
	}
	Sizuku_->SetShininess(20.0f);
}

void PlayerModels::Update() {

	const char* desiredAnimationName = "Idle";
	bool loopAnimation = true;
	switch (state_) {
	case PlayerModels::idle:
		desiredAnimationName = "Idle";
		loopAnimation = true;
		break;
	case PlayerModels::walk:
		desiredAnimationName = "Walk";
		loopAnimation = true;
		break;
	case PlayerModels::attack1:
		desiredAnimationName = "Attack1";
		loopAnimation = false;
		break;
	case PlayerModels::attack2:
		desiredAnimationName = "Attack2";
		loopAnimation = false;
		break;
	case PlayerModels::attack3:
		desiredAnimationName = "Attack3";
		loopAnimation = false;
		break;
	case PlayerModels::attack4:
		desiredAnimationName = "Attack4";
		loopAnimation = false;
		break;
	case PlayerModels::fallingAttack:
		desiredAnimationName = "FallAttack";
		loopAnimation = false;
		break;
	case PlayerModels::skillAttack:
		desiredAnimationName = "SkillAttack";
		loopAnimation = false;
		break;
	case PlayerModels::damage:
		desiredAnimationName = "damage";
		loopAnimation = true;
		break;
	default:
		break;
	}
	const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
	AnimationManager::PlaybackResult playbackResult{};
	if (AnimationManager::GetInstance()->UpdatePlayback(animationGroupName_, desiredAnimationName, loopAnimation, deltaTime, kAnimationBlendDuration_, blendedPoseAnimation_, playbackResult)) {
		animationFinished_ = playbackResult.animationFinished;
		if (playbackResult.changedAnimation && playbackResult.currentAnimation) {
			Sizuku_->SetAnimation(playbackResult.currentAnimation, loopAnimation);
		}

		if (sizukuSkeleton_ && playbackResult.animationToApply) {
			sizukuSkeleton_->ApplyAnimation(*playbackResult.animationToApply, playbackResult.animationTime);
			sizukuSkeleton_->Update();
			if (!sizukuSkinCluster_.mappedPalette.empty()) {
				UpdateSkinCluster(sizukuSkinCluster_, *sizukuSkeleton_);
			}
		}
	}
	playerWorld = Function::MakeAffineMatrix(player_.scale, player_.rotate, player_.translate);

	Sizuku_->SetWorldMatrix(playerWorld);
	Sizuku_->SetCamera(camera_);

	Sizuku_->Update();

#ifdef USE_IMGU
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


	Object3dCommon::GetInstance()->DrawCommonSkinningToon();
	Sizuku_->Draw();

#ifdef _DEBUG
	if (sizukuSkeleton_) {
		sizukuSkeleton_->SetObjectMatrix(playerWorld);
		Object3dCommon::GetInstance()->DrawCommonWireframeNoDepth();
		/*sizukuSkeleton_->DrawBones(camera_, {0.2f, 0.9f, 1.0f, 1.0f}, {0.1f, 0.5f, 0.9f, 1.0f});*/
	}
#endif
}
std::optional<Matrix4x4> PlayerModels::GetJointWorldMatrix(const std::string& jointName) const {
	if (!sizukuSkeleton_) {
		return std::nullopt;
	}

	const auto jointIndex = sizukuSkeleton_->FindJointIndex(jointName);
	if (!jointIndex.has_value()) {
		return std::nullopt;
	}

	const auto& joints = sizukuSkeleton_->GetJoints();
	if (*jointIndex < 0 || static_cast<size_t>(*jointIndex) >= joints.size()) {
		return std::nullopt;
	}

	const Matrix4x4 playerWorld = Function::MakeAffineMatrix(player_.scale, player_.rotate, player_.translate);
	return Function::Multiply(joints[*jointIndex].skeletonSpaceMatrix, playerWorld);
}