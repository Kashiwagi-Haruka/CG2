#include "Desk.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/Player/Player.h"
#include"Object3d/Object3dCommon.h"
#include"Animation/AnimationManager.h"
#include "GameBase.h"

PlayerCamera* Desk::playerCamera_ = nullptr;

Desk::Desk()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/desk", "desk");
    obj_->SetModel("desk");
    SetAABB({ .min = {-0.4f,0.0f,-0.3f},.max = {0.4f,0.8f,0.3f} });
    SetCollisionAttribute(kCollisionDesk);
    SetCollisionMask(kCollisionPlayer | kCollisionKey|kCollisionDesk);
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
}

void Desk::Initialize()
{
    obj_->Initialize();

    AnimationManager::GetInstance()->LoadAnimationGroup(animationGroupName_, "Resources/TD3_3102/3d/desk", "desk");
    AnimationManager::GetInstance()->ResetPlayback(animationGroupName_, "Open", true);
    if (const Animation::AnimationData* idleAnimation = AnimationManager::GetInstance()->FindAnimation(animationGroupName_, "Open")) {
        obj_->SetAnimation(idleAnimation, true);
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

    if (OnCollisionRay()) {
        //rayの当たり判定
        if (PlayerCommand::GetInstance()->InteractTrigger()) {
            if (!PlayerCommand::GetIsGrab()) {

            }
        }
    }

}


bool Desk::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(GetAABB(), GetWorldPosition());
}
