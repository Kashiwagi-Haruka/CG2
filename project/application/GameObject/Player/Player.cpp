#include "Player.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"

#include "Model/ModelManager.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"
#include"GameObject/KeyBindConfig.h"
#include<imgui.h>
#include"GameObject/YoshidaMath/Easing.h"
namespace PlayerConst{
    const constexpr float kRotateYSpeed = 0.25f;
    const constexpr float kSneakSpeed = 0.125f;
    const constexpr float kWalkSpeed = 0.25f;
};

Player::Player()
{
    //体のObject3d
    bodyObj_ = std::make_unique<Object3d>();
    //モデルの読み込み
    ModelManager::GetInstance()->LoadGltfModel("Resources/3d/human", "walk");
    ModelManager::GetInstance()->LoadGltfModel("Resources/3d/human", "sneakWalk");
}
void Player::SetCamera(Camera* camera)
{
    bodyObj_->SetCamera(camera);
}
void Player::Initialize()
{
    bodyObj_->Initialize();

    bodyObj_->SetModel("walk");

    transform_ = {
    .scale{100.0f,100.0f,100.0f},
    .rotate{-YoshidaMath::PI / 2.0f, 0.0f, 0.0f  },
    .translate{0.0f,1.0f,-3.0f}
    };

    velocity_ = { 0.0f,0.0f,0.0f };
    speed_ = { 0.0f };


    //アニメーションクリップ
    animationClips_ = Animation::LoadAnimationClips("Resources/3d/human", "walk");
    std::vector<Animation::AnimationData> sneakClips = Animation::LoadAnimationClips("Resources/3d/human", "sneakWalk");
    animationClips_.insert(animationClips_.end(), sneakClips.begin(), sneakClips.end());

    if (!animationClips_.empty()) {
        currentAnimationIndex_ = 0;
        bodyObj_->SetAnimation(&animationClips_[currentAnimationIndex_], true);
    }


    if (Model* walkModel = ModelManager::GetInstance()->FindModel("walk")) {
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

    bodyObj_->SetTransform(transform_);
    bodyObj_->Update();
    //アニメーション
    Animation();
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
    Vector2 direction = YoshidaMath::Normalize({ velocity_.x,velocity_.z });
    // Y軸回転（左右）
    transform_.rotate.y = YoshidaMath::Easing::Lerp(transform_.rotate.y, std::atan2(direction.x, direction.y), PlayerConst::kRotateYSpeed);

    float length = YoshidaMath::Length(Vector2{ velocity_.x,velocity_.z });
    speed_ = (playerCommand->Sneak() || length <= 0.5f) ? PlayerConst::kSneakSpeed: PlayerConst::kWalkSpeed;

    if (fabs(velocity_.x) > 0.0f || fabs(velocity_.z) > 0.0f) {

        //前の方向を取得
        Vector3 forward = YoshidaMath::GetForward(transform_.rotate.z);
        forward.y = 0.0f;

        // forwardに垂直な右方向ベクトルを計算
        Vector3 right = Function::Cross(Vector3(0, 1, 0), forward);
        right = Function::Normalize(right);
        //速度を正規化しそれぞれ足す

        transform_.translate += forward * direction.y * speed_;
        transform_.translate += right * direction.x * speed_;
    }


}

void Player::Animation()
{
    float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();


    auto* playerCommand = PlayerCommand::GetInstance();
    if (playerCommand->Sneak()) {
        currentAnimationIndex_ = 1;
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
