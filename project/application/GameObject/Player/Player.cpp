#include "Player.h"
#include"Function.h"
#include<numbers>
#include "Model/ModelManager.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"
#include"KeyBindConfig.h"
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
    Move();

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

    Vector2 direction = MyMath::Normalize({ velocity_.x,velocity_.z });

    // Y軸回転（左右）
    transform_.rotate.y = std::atan2(direction.x, direction.y);

   /* transform_.rotate = Function::DirectionToRotation(Function::Normalize({ velocity_.x,0.0f,velocity_.z }), { 1,0,0 });*/

    float length = MyMath::Length(Vector2{ velocity_.x,velocity_.z });
    speed_ = (playerCommand->Sneak() || length >= 0.5f) ? 0.25f : 0.125f;

    if (fabs(velocity_.x) > 0.0f || fabs(velocity_.z) > 0.0f) {

        //前の方向を取得
        Vector3 forward = MyMath::GetForward(transform_.rotate.z);
        forward.y = 0.0f;

        // forwardに垂直な右方向ベクトルを計算
        Vector3 right = Function::Cross(Vector3(0, 1, 0), forward);
        right = Function::Normalize(right);
        //速度を正規化しそれぞれ足す
// x, z 成分だけ正規化 
        Vector3 horizontal = Function::Normalize(Vector3{ velocity_.x, 0.0f, velocity_.z });

        transform_.translate += forward * horizontal.z * speed_;
        transform_.translate += right * horizontal.x * speed_;
    }


}

float MyMath::Dot(const Vector2& v1, const Vector2& v2)
{
    return { v1.x * v2.x + v1.y * v2.y };
}

float MyMath::Length(const Vector2& v)
{
    return { sqrtf(Dot(v,v)) };
}

Vector2 MyMath::Normalize(const Vector2& v)
{
    float length = Length(v);
    if (length != 0.0f) {
        return { v.x / length,v.y / length };
    } else {
        return { 0.0f, 0.0f };
    }
}

Vector3 MyMath::GetForward(const float angle) {

    Matrix4x4 rotationZMatrix;

    rotationZMatrix = Function::MakeRotateZMatrix(angle);
    Vector3 localForward = { 0.0f, 0.0f, 1.0f };
    return Function::TransformVM(localForward, rotationZMatrix);
}

