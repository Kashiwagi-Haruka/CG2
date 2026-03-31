#include "Desk.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/Player/Player.h"
#include"Object3d/Object3dCommon.h"
#include"Animation/AnimationManager.h"
#include "GameBase.h"
#include"GameObject/SEManager/SEManager.h"

PlayerCamera* Desk::playerCamera_ = nullptr;

Desk::Desk()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/desk", "desk");
    obj_->SetModel("desk");
    SetAABB({ .min = {-0.4f,0.0f,-0.3f},.max = {0.4f,0.8f,0.3f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer | kCollisionKey | kCollisionItem);
    localAABB_ = { .min = {-0.125f,-0.125f,-0.125f},.max = {0.125f,0.125,0.125f} };
}

void Desk::OnCollision(Collider* collider)
{

}

Vector3 Desk::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}

void Desk::Animation()
{


    bool loopAnimation = false;

    if (desiredAnimationName == "Close") {
        if (animationFinished_) {
            desiredAnimationName = "Idle";
        }
    }

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

void Desk::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}

void Desk::Update()
{

    CheckCollision();
    Animation();
    obj_->Update();

    if (!isStart_) {
        isStart_ = true;
    }
}

void Desk::Initialize()
{
    obj_->Initialize();
    
    isStart_ = false;
    desiredAnimationName = "Idle";

    AnimationManager::GetInstance()->LoadAnimationGroup(animationGroupName_, "Resources/TD3_3102/3d/desk", "desk");
    AnimationManager::GetInstance()->ResetPlayback(animationGroupName_, desiredAnimationName, false);
    if (const Animation::AnimationData* idleAnimation = AnimationManager::GetInstance()->FindAnimation(animationGroupName_, desiredAnimationName)) {
        obj_->SetAnimation(idleAnimation, false);
    }

    if (Model* sizukuModel = ModelManager::GetInstance()->FindModel("desk")) {
        skeleton_ = std::make_unique<Skeleton>(Skeleton().Create(sizukuModel->GetModelData().rootnode));
        skinCluster_ = CreateSkinCluster(*skeleton_, *sizukuModel);
        if (!skinCluster_.mappedPalette.empty()) {
            obj_->SetSkinCluster(&skinCluster_);
        }
    }
}

void Desk::Draw()
{
    obj_->Draw();
}


void Desk::CheckCollision()
{
    if (!isStart_) {
        return;
    }

    if (OnCollisionRay()) {
        //rayの当たり判定
        if (PlayerCommand::GetInstance()->InteractTrigger()) {
            if (!PlayerCommand::GetIsGrab()) {

    /*                SEManager::SoundPlay(SEManager::CHAIR);*/
                    if (desiredAnimationName == "Idle") {
                        desiredAnimationName = "Open";
                    } else if (desiredAnimationName == "Open") {
                        desiredAnimationName = "Idle";
                    }
                

            }
        }
    }

}


bool Desk::OnCollisionRay()
{
    const std::optional<int32_t> jointIndex = skeleton_->FindJointIndex("drawer.002");
    skeleton_->SetObjectMatrix(obj_->GetWorldMatrix());
    Vector3 pos = skeleton_->GetJointWorldPosition(skeleton_->GetJoints()[*jointIndex]);
    return playerCamera_->OnCollisionRay(localAABB_, pos);
}
