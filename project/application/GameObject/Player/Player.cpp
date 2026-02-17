#include "Player.h"
#include"Input.h"
#include"Function.h"
#include<numbers>
#include "Model/ModelManager.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"

#include<imgui.h>
namespace {
    const constexpr float PI = std::numbers::pi_v<float>;
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
    .rotate{-PI / 2.0f, 0.0f, 0.0f  },
    .translate{0.0f,1.0f,-3.0f}
    };

    velocity_ = { 0.0f,0.0f,0.0f };

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
    auto* input = Input::GetInstance();

    if (input->TriggerKey(DIK_W)) {
        Function::DirectionToRotation({ 0,0,1 }, { 1,0,0 });
    }

    if (input->TriggerKey(DIK_S)) {
        Function::DirectionToRotation({ 0,0,-1 }, { 1,0,0 });
    }

    if (input->TriggerKey(DIK_A)) {
        Function::DirectionToRotation({ -1,0,0 }, { 1,0,0 });
    }

    if (input->TriggerKey(DIK_D)) {
        Function::DirectionToRotation({ 1,0,0 }, { 1,0,0 });
    }

    bodyObj_->SetTransform(transform_);
    bodyObj_->Update();

    float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();

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
