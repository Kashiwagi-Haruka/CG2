#define NOMINMAX
#include "Player.h"

#include "Animation/Animation.h"
#include "DirectXCommon.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "GameObject/KeyBindConfig.h"
#include "GameObject/YoshidaMath/Easing.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include <algorithm>
#include <imgui.h>
#include <optional>

namespace {
struct FootContactState {
	bool left = false;
	bool right = false;
	float leftY = 0.0f;
	float rightY = 0.0f;
	float groundY = 0.0f;
};

float WrapAnimationTime(float time, float duration) {
	if (duration <= 0.0f) {
		return 0.0f;
	}

	while (time < 0.0f) {
		time += duration;
	}
	while (time >= duration) {
		time -= duration;
	}
	return time;
}

std::optional<FootContactState> SampleFootContactState(
    const Animation::AnimationData& animation, float sampleTime, const Matrix4x4& objectMatrix, const Model& model, const char* leftJointName = "foot.L", const char* rightJointName = "foot.R") {
	Skeleton skeleton = Skeleton().Create(model.GetModelData().rootnode);
	skeleton.ApplyAnimation(animation, WrapAnimationTime(sampleTime, animation.duration));
	skeleton.Update();
	skeleton.SetObjectMatrix(objectMatrix);

	const std::optional<int32_t> leftIndex = skeleton.FindJointIndex(leftJointName);
	const std::optional<int32_t> rightIndex = skeleton.FindJointIndex(rightJointName);
	if (!leftIndex.has_value() || !rightIndex.has_value()) {
		return std::nullopt;
	}

	const std::vector<Joint>& joints = skeleton.GetJoints();
	const Vector3 leftPosition = skeleton.GetJointWorldPosition(joints[*leftIndex]);
	const Vector3 rightPosition = skeleton.GetJointWorldPosition(joints[*rightIndex]);
	const float groundY = std::min(leftPosition.y, rightPosition.y);
	const float contactThreshold = 0.02f;

	return FootContactState{
	    .left = std::abs(leftPosition.y - groundY) <= contactThreshold,
	    .right = std::abs(rightPosition.y - groundY) <= contactThreshold,
	    .leftY = leftPosition.y,
	    .rightY = rightPosition.y,
	    .groundY = groundY,
	};
}
} // namespace

Player::Player() {
    localAABB_ = {
        .min = {-0.25f, 0.0f, -0.25f},
          .max = {0.25f,  1.5f, 0.25f }
    };
    SetAABB(localAABB_);
    SetCollisionAttribute(kCollisionPlayer);
    SetCollisionMask(kCollisionFloor | kCollisionPortal | kCollisionEnemy | kCollisionItem | kCollisionKey | kCollisionChair | kCollisionWall | kCollisionVendingMac | kCollisionDoor | kCollisionMat);
    // 体のObject3d
    bodyObj_ = std::make_unique<Object3d>();
    // モデルの読み込み
    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/gentleman", "gentleman");

    footStepSE = Audio::GetInstance()->SoundLoadFile("Resources/TD3_3102/Audio/SE/maou_se_sound_footstep02.mp3");
}
void Player::SetCamera(Camera* camera) {
    // カメラのセット
    bodyObj_->SetCamera(camera);
    bodyObj_->UpdateCameraMatrices();
}
void Player::Initialize() {

    // 体の初期化
    bodyObj_->Initialize();
    // 体にモデル挿入
    bodyObj_->SetModel("gentleman");
    // 座標の初期化
    transform_ = {
        .scale{1.0f, 1.0f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{0.0f, 2.0f, 0.0f}
    };
    // 速度の初期化
    velocity_ = { 0.0f };
    forward_ = { 0.0f };
    soundTimer_ = 0.0f;
    moveSpeed_ = { 0.0f };

    LoadParameters();

    // アニメーションクリップ
    animationClips_ = Animation::LoadAnimationClips("Resources/TD3_3102/3d/gentleman", "gentleman");

    if (!animationClips_.empty()) {
		currentAnimationIndex_ = FindAnimationIndex("Walk", 0);
		bodyObj_->SetAnimation(&animationClips_[currentAnimationIndex_], true);
	}


    if (Model* walkModel = ModelManager::GetInstance()->FindModel("gentleman")) {
        skeleton_ = std::make_unique<Skeleton>(Skeleton().Create(walkModel->GetModelData().rootnode));
        skinCluster_ = CreateSkinCluster(*skeleton_, *walkModel);
        if (!skinCluster_.mappedPalette.empty()) {
            bodyObj_->SetSkinCluster(&skinCluster_);
        }
    }
}

void Player::Update()
{
	const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
	soundTimer_ = std::max(0.0f, soundTimer_ - deltaTime);

	ResetFootContactState();
    //移動処理
    Move();
    //重力処理
    Gravity();
    //アニメーション
    Animation();
    bodyObj_->SetTransform(transform_);
    bodyObj_->Update();

    //デバック
    Debug();
}

void Player::Draw()
{

    bodyObj_->Draw();

}

void Player::Debug() {
#ifdef USE_IMGUI
	if (ImGui::Begin("Human")) {

		if (ImGui::TreeNode("Transform")) {
			ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f);
			ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.1f);
			ImGui::DragFloat3("Translate", &transform_.translate.x, 0.1f);
			ImGui::TreePop();
		}
		if (!animationClips_.empty()) {
			std::vector<const char*> animationNames;
			animationNames.reserve(animationClips_.size());
			for (const auto& clip : animationClips_) {
				animationNames.push_back(clip.name.c_str());
			}
			int selectedIndex = static_cast<int>(currentAnimationIndex_);
			if (ImGui::Combo("Animation", &selectedIndex, animationNames.data(), static_cast<int>(animationNames.size()))) {
				currentAnimationIndex_ = static_cast<size_t>(selectedIndex);
				bodyObj_->SetAnimation(&animationClips_[currentAnimationIndex_], true);
				animationTime_ = 0.0f;
			}

			if (Model* playerModel = ModelManager::GetInstance()->FindModel("gentleman")) {
				const Animation::AnimationData& currentAnimation = animationClips_[currentAnimationIndex_];
				const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
				const float previousTime = WrapAnimationTime(animationTime_ - deltaTime, currentAnimation.duration);
				const float nextTime = WrapAnimationTime(animationTime_ + deltaTime, currentAnimation.duration);

				const std::optional<FootContactState> previousContact = SampleFootContactState(currentAnimation, previousTime, bodyObj_->GetWorldMatrix(), *playerModel);
				const std::optional<FootContactState> nextContact = SampleFootContactState(currentAnimation, nextTime, bodyObj_->GetWorldMatrix(), *playerModel);

				ImGui::Separator();
				ImGui::Text("Foot Contact");
				if (previousContact.has_value()) {
					ImGui::Text("Prev Frame  L:%s  R:%s", previousContact->left ? "Ground" : "Air", previousContact->right ? "Ground" : "Air");
					ImGui::TextDisabled("Prev Y  L:%.3f R:%.3f Ground:%.3f", previousContact->leftY, previousContact->rightY, previousContact->groundY);
				} else {
					ImGui::TextDisabled("Prev Frame: foot joint not found");
				}

				if (nextContact.has_value()) {
					ImGui::Text("Next Frame  L:%s  R:%s", nextContact->left ? "Ground" : "Air", nextContact->right ? "Ground" : "Air");
					ImGui::TextDisabled("Next Y  L:%.3f R:%.3f Ground:%.3f", nextContact->leftY, nextContact->rightY, nextContact->groundY);
				} else {
					ImGui::TextDisabled("Next Frame: foot joint not found");
				}
			}
		}

		if (ImGui::TreeNode("Parameters")) {
			ImGui::DragFloat("Rotate Speed", &parameters_.kRotateYSpeed, 0.001f, 0.0f, 10.0f);
			ImGui::DragFloat("Walk Speed", &parameters_.kWalkSpeed, 0.001f, 0.0f, 10.0f);
			ImGui::DragFloat("Sneak Speed", &parameters_.kSneakSpeed, 0.001f, 0.0f, 10.0f);

			if (ImGui::Button("Save Player Parameters")) {
				SaveParameters();
			}
			ImGui::SameLine();
			if (ImGui::Button("Load Player Parameters")) {
				LoadParameters();
			}
			if (!parameterStatusMessage_.empty()) {
				ImGui::Text("%s", parameterStatusMessage_.c_str());
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();
#endif
}


void Player::Move()
{

    velocity_.x = { 0.0f };
    velocity_.z = { 0.0f };

    auto* input = Input::GetInstance();

    Vector2 controllerPos = input->GetJoyStickLXY();

    velocity_.x = controllerPos.x;
    velocity_.z = controllerPos.y;

    auto* playerCommand = PlayerCommand::GetInstance();

    if (fabs(velocity_.x) <= 0.0f && fabs(velocity_.z) <= 0.0f) {
        if (playerCommand->MoveLeft()) { velocity_.x = -1.0f; }
        if (playerCommand->MoveRight()) { velocity_.x = 1.0f; }
        if (playerCommand->MoveForward()) { velocity_.z = 1.0f; }
        if (playerCommand->MoveBackward()) { velocity_.z = -1.0f; }
    }

    //xy成分だけ正規化
    Vector3 horizontal = Function::Normalize({ velocity_.x,0.0f,velocity_.z });
    //Y回転
    float yaw = std::atan2(horizontal.x, horizontal.y);
    //ベクトルのXZ長さ
    float length = YoshidaMath::Length(Vector2{ velocity_.x,velocity_.z });
    moveSpeed_ = (playerCommand->Sneak() || length <= 0.5f) ? parameters_.kSneakSpeed : parameters_.kWalkSpeed;
    //前の方向を取得
    forward_ = YoshidaMath::GetForward(bodyObj_->GetWorldMatrix());

    if (fabs(velocity_.x) > 0.0f || fabs(velocity_.z) > 0.0f) {
        Vector3 forward = forward_;
        forward.y = 0.0f;

        // forwardに垂直な右方向ベクトルを計算
        Vector3 right = Function::Cross({ 0.0f, 1.0f, 0.0f }, forward);
        right = Function::Normalize(right);
        //速度を正規化しそれぞれ足す
        transform_.translate += forward * horizontal.z * moveSpeed_;
        transform_.translate += right * horizontal.x * moveSpeed_;


    }
}

void Player::ResetFootContactState() {
	leftFootGrounded_ = false;
	rightFootGrounded_ = false;
}

void Player::UpdateFootContact(Collider* collider) {
	if (!collider) {
		return;
	}

	const uint32_t collisionAttribute = collider->GetCollisionAttribute();
	if (collisionAttribute != kCollisionFloor && collisionAttribute != kCollisionMat) {
		return;
	}

	const bool wasLeftFootGrounded = leftFootGrounded_;
	const bool wasRightFootGrounded = rightFootGrounded_;

	leftFootGrounded_ = leftFootGrounded_ || CheckFootContact(collider, kLeftFootJointName);
	rightFootGrounded_ = rightFootGrounded_ || CheckFootContact(collider, kRightFootJointName);

	if ((!wasLeftFootGrounded && leftFootGrounded_) || (!wasRightFootGrounded && rightFootGrounded_)) {
		PlayFootstepSE();
	}
}

bool Player::CheckFootContact(Collider* collider, const char* jointName) const {
	if (!collider || !skeleton_) {
		return false;
	}

	const std::optional<int32_t> jointIndex = skeleton_->FindJointIndex(jointName);
	if (!jointIndex.has_value()) {
		return false;
	}

	skeleton_->SetObjectMatrix(bodyObj_->GetWorldMatrix());
	const Vector3 footPosition = skeleton_->GetJointWorldPosition(skeleton_->GetJoints()[*jointIndex]);

	const AABB colliderLocalAABB = collider->GetAABB();
	const Vector3 colliderPosition = collider->GetWorldPosition();
	const AABB colliderWorldAABB = {
	    .min = colliderLocalAABB.min + colliderPosition,
	    .max = colliderLocalAABB.max + colliderPosition,
	};

	const AABB footAABB = {
	    .min = {footPosition.x - kFootContactHalfWidth, footPosition.y - kFootContactHeight, footPosition.z - kFootContactHalfWidth},
	    .max = {footPosition.x + kFootContactHalfWidth, footPosition.y,                      footPosition.z + kFootContactHalfWidth},
	};

	return YoshidaMath::IsCollision(footAABB, colliderWorldAABB);
}

bool Player::IsMovingHorizontally() const { return std::abs(velocity_.x) > 0.01f || std::abs(velocity_.z) > 0.01f; }
size_t Player::FindAnimationIndex(const char* animationName, size_t fallbackIndex) const {
	const auto found =
	    std::find_if(animationClips_.begin(), animationClips_.end(), [animationName](const Animation::AnimationData& clip) { return clip.name.find(animationName) != std::string::npos; });
	if (found == animationClips_.end()) {
		return std::min(fallbackIndex, animationClips_.empty() ? size_t{0} : animationClips_.size() - 1);
	}
	return static_cast<size_t>(std::distance(animationClips_.begin(), found));
}

void Player::UpdateMovementAnimation(bool isSneaking) {
	if (animationClips_.empty()) {
		return;
	}

	const size_t desiredAnimationIndex = isSneaking ? FindAnimationIndex("SneakWalk", 0) : FindAnimationIndex("Walk", 0);
	if (currentAnimationIndex_ == desiredAnimationIndex) {
		return;
	}

	currentAnimationIndex_ = desiredAnimationIndex;
	animationTime_ = 0.0f;
	bodyObj_->SetAnimation(&animationClips_[currentAnimationIndex_], true);
}
void Player::PlayFootstepSE() {
	if (!IsMovingHorizontally() || soundTimer_ > 0.0f) {
		return;
	}

	const bool isWalking = moveSpeed_ == parameters_.kWalkSpeed;
	Audio::GetInstance()->SetSoundVolume(&footStepSE, (moveSpeed_ == parameters_.kWalkSpeed) ? 0.5f : 0.25f);
	Audio::GetInstance()->SoundPlayWave(footStepSE);
	soundTimer_ = isWalking ? kWalkFootstepInterval : kSneakFootstepInterval;
}

void Player::Gravity() {
	velocity_.y -= YoshidaMath::kDeltaTime * YoshidaMath::kGravity;
	transform_.translate.y += velocity_.y;
	velocity_.y = std::clamp(velocity_.y, -1.0f, 1.0f);
}

void Player::OnCollision(Collider* collider) {
	UpdateFootContact(collider);

	if (collider->GetCollisionAttribute() != kCollisionMat) {
		// マットじゃなかったら
		OnCollisionObstacle();
	}
	if (collider->GetCollisionAttribute() == kCollisionPortal) {
	}
}

Vector3 Player::GetWorldPosition() const
{
    return bodyObj_->GetTranslate();
}

void Player::OnCollisionObstacle()
{
    YoshidaMath::ResolveCollision(transform_.translate, velocity_, GetCollisionInfo());
}

void Player::Animation()
{
    float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();

    auto* playerCommand = PlayerCommand::GetInstance();
	UpdateMovementAnimation(playerCommand->Sneak() || moveSpeed_ == parameters_.kSneakSpeed);


    if (skeleton_ && !animationClips_.empty()) {
        const Animation::AnimationData& currentAnimation = animationClips_[currentAnimationIndex_];
        animationTime_ = Animation::AdvanceTime(currentAnimation, animationTime_, deltaTime, true);
        skeleton_->ApplyAnimation(currentAnimation, animationTime_);
        skeleton_->Update();
        if (!skinCluster_.mappedPalette.empty()) {
            UpdateSkinCluster(skinCluster_, *skeleton_);
        }
        Matrix4x4 humanWorld = bodyObj_->GetWorldMatrix();
        skeleton_->SetObjectMatrix(humanWorld);
    }
}
void Player::SaveParameters() {
    JsonManager* jsonManager = JsonManager::GetInstance();

    nlohmann::json root;
    root["playerParameters"] = {
        {"rotateYSpeed", parameters_.kRotateYSpeed},
        {"walkSpeed",    parameters_.kWalkSpeed   },
        {"sneakSpeed",   parameters_.kSneakSpeed  },
    };

    jsonManager->SetData(root);
    const bool saved = jsonManager->SaveJson(kParameterFileName);
    parameterStatusMessage_ = saved ? "Saved: Resources/JSON/playerParameters.json" : "Save failed: Resources/JSON/playerParameters.json";
}

void Player::LoadParameters() {
    JsonManager* jsonManager = JsonManager::GetInstance();

    if (!jsonManager->LoadJson(kParameterFileName)) {
        parameterStatusMessage_ = "Load failed: Resources/JSON/playerParameters.json";
        return;
    }

    const nlohmann::json& root = jsonManager->GetData();
    if (!root.contains("playerParameters") || !root["playerParameters"].is_object()) {
        parameterStatusMessage_ = "Load failed: invalid player parameter data";
        return;
    }

    const nlohmann::json& params = root["playerParameters"];

    if (params.contains("rotateYSpeed") && params["rotateYSpeed"].is_number()) {
        parameters_.kRotateYSpeed = params["rotateYSpeed"].get<float>();
    }
    if (params.contains("walkSpeed") && params["walkSpeed"].is_number()) {
        parameters_.kWalkSpeed = params["walkSpeed"].get<float>();
    }
    if (params.contains("sneakSpeed") && params["sneakSpeed"].is_number()) {
        parameters_.kSneakSpeed = params["sneakSpeed"].get<float>();
    }

    parameterStatusMessage_ = "Loaded player parameters";
}