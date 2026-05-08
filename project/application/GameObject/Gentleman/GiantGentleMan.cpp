#include "GiantGentleMan.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"Model/ModelManager.h"
#include"Animation/AnimationManager.h"
#include "GameBase.h"
#include"GameObject/SEManager/SEManager.h"
#include<imgui.h>
#include"GameSave/GameSave.h"

PlayerCamera* GiantGentleMan::playerCamera_ = nullptr;
bool GiantGentleMan::isRayHit_ = false;


GiantGentleMan::GiantGentleMan()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/gentleman", "gentleman");
    obj_->SetModel("gentleman");
    localAABB_ = { .min = {-0.5f,-0.5f,-0.5f},.max = {0.5f,0.5,0.5f} };

    head_ = std::make_unique<GiantGentlemanHead>();

    colliders_["GentlemanLeftHand"] = std::make_unique<GitantGettlemanCollider>();
    colliders_["GentlemanRightHand"] = std::make_unique<GitantGettlemanCollider>();
    colliders_["GentlemanShoulder.L"] = std::make_unique<GitantGettlemanCollider>();
    colliders_["GentlemanShoulder.R"] = std::make_unique<GitantGettlemanCollider>();
    colliders_["GentlemanLeftArm"] = std::make_unique<GitantGettlemanCollider>();
    colliders_["GentlemanRightArm"] = std::make_unique<GitantGettlemanCollider>();
}

void GiantGentleMan::Initialize()
{
    obj_->Initialize();
    obj_->RegisterEditor("giantGentleMan");

    isRayHit_ = false;

    AnimationManager::GetInstance()->LoadAnimationGroup(animationGroupName_, "Resources/TD3_3102/3d/gentleman", "gentleman");
    AnimationManager::GetInstance()->ResetPlayback(animationGroupName_, desiredAnimationName, false);
    if (const Animation::AnimationData* idleAnimation = AnimationManager::GetInstance()->FindAnimation(animationGroupName_, desiredAnimationName)) {
        obj_->SetAnimation(idleAnimation, false);
    }

    if (Model* sizukuModel = ModelManager::GetInstance()->FindModel("gentleman")) {
        skeleton_ = std::make_unique<Skeleton>(Skeleton().Create(sizukuModel->GetModelData().rootnode));
        skinCluster_ = CreateSkinCluster(*skeleton_, *sizukuModel);
        if (!skinCluster_.mappedPalette.empty()) {
            obj_->SetSkinCluster(&skinCluster_);
        }
    }

    head_->Initialize();

    for (auto& [name, hand] : colliders_) {
        hand->RegisterEditor(name);
    }

    colliders_["GentlemanLeftHand"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["GentlemanRightHand"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["GentlemanShoulder.L"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["GentlemanShoulder.R"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["GentlemanLeftArm"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["GentlemanRightArm"]->Initialize(YoshidaMath::ColliderType::kAABB);
}

void GiantGentleMan::Update()
{
    CheckCollision();
    Animation();
    obj_->Update();
    head_->Update();
    for (auto& [name, hand] : colliders_) {
        hand->Update();
    }
}

void GiantGentleMan::Draw()
{
    obj_->Draw();
    //head_->Draw();
    //for (auto& [name, hand] : colliders_) {
    //    hand->Draw();
    //}
}
void GiantGentleMan::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();

    head_->SetCamera(camera);
    for (auto& [name, hand] : colliders_) {
        hand->SetCamera(camera);
    }
}
bool GiantGentleMan::IsFacingSurface(const Matrix4x4& cameraMat)
{
    Vector3 forward = YoshidaMath::GetForward(cameraMat);
    Vector3 direction = YoshidaMath::GetForward(obj_->GetWorldMatrix());
    float dot = Function::Dot(forward, direction);

    //return(fabs(dot) >= kPortalCreatableAngleRange_);
    return(dot <= -kPortalCreatableAngleRange_);
}
void GiantGentleMan::CheckCollision()
{
    isRayHit_ = OnCollisionRay();
    //rayの当たり判定
    if (isRayHit_ && PlayerCommand::GetInstance()->Shot()) {
        //トリガーしたとき且つ何も持ってないとき
        SEManager::SoundPlay(SEManager::TYPE);
    }


#ifdef USE_IMGUI
    ImGui::Begin("GiantGentleMan");
    ImGui::Text("CurrantAnimation : %s", desiredAnimationName.c_str());
    ImGui::End();

#endif
}

bool GiantGentleMan::OnCollisionRay()
{
    //舌のコントロールボーン
    const std::optional<int32_t> jointIndex = skeleton_->FindJointIndex("CON.tongue.002");
    skeleton_->SetObjectMatrix(obj_->GetWorldMatrix());
    Vector3 pos = skeleton_->GetJointWorldPosition(skeleton_->GetJoints()[*jointIndex]);
    return playerCamera_->OnCollisionRay(localAABB_, pos);
}


void GiantGentleMan::Animation()
{
    bool loopAnimation = true;
    const float deltaTime = GameBase::GetInstance()->GetDeltaTime();
    AnimationManager::PlaybackResult playbackResult{};

    if (AnimationManager::GetInstance()->UpdatePlayback(animationGroupName_, desiredAnimationName, loopAnimation, deltaTime, kAnimationBlendDuration_, blendedPoseAnimation_, playbackResult)) {
        animationFinished_ = playbackResult.animationFinished;

        if (playbackResult.changedAnimation && playbackResult.currentAnimation) {
            obj_->SetAnimation(playbackResult.currentAnimation, loopAnimation);
        }

        if (skeleton_ && playbackResult.animationToApply) {
            skeleton_->ApplyAnimation(*playbackResult.animationToApply, playbackResult.animationTime);
            skeleton_->Update();
            if (!skinCluster_.mappedPalette.empty()) {
                UpdateSkinCluster(skinCluster_, *skeleton_);
            }
        }
    }
}
