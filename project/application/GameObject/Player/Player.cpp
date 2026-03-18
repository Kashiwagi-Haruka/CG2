#include "Player.h"

#include "Animation/Animation.h"
#include "DirectXCommon.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "GameObject/KeyBindConfig.h"
#include "GameObject/YoshidaMath/Easing.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include <imgui.h>

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
        currentAnimationIndex_ = 0;
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

void Player::Debug()
{
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

void Player::PlayFootstepSE() {
	Audio::GetInstance()->SoundPlayWave(footStepSE);
	Audio::GetInstance()->SetSoundVolume(&footStepSE, (moveSpeed_ == parameters_.kWalkSpeed) ? 0.5f : 0.25f);
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
    if (playerCommand->Sneak()) {
        //currentAnimationIndex_ = 1;
    } else {
        currentAnimationIndex_ = 0;
    }


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