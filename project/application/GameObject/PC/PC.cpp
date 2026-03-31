#include "PC.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/Player/Player.h"
#include"Object3d/Object3dCommon.h"
#include"Animation/AnimationManager.h"
#include "GameBase.h"
#include"GameObject/SEManager/SEManager.h"

PlayerCamera* PC::playerCamera_ = nullptr;

PC::PC()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/pc", "pc");
    obj_->SetModel("pc");
    SetAABB({ .min = {-0.15f,-0.15f,-0.15f},.max = {0.15f,0.15f,0.15f} });
    SetCollisionAttribute(kCollisionItem);
    SetCollisionMask(kCollisionPlayer | kCollisionKey | kCollisionWall);
}

void PC::OnCollision(Collider* collider)
{

}

Vector3 PC::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}

void PC::Animation()
{

    bool loopAnimation = false;

    if (desiredAnimationName == "Open") {
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

void PC::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}

void PC::Update()
{

    CheckCollision();
    Animation();
    obj_->Update();
}

void PC::Initialize()
{
    obj_->Initialize();

    AnimationManager::GetInstance()->LoadAnimationGroup(animationGroupName_, "Resources/TD3_3102/3d/pc", "pc");
    AnimationManager::GetInstance()->ResetPlayback(animationGroupName_, "Idle", false);
    if (const Animation::AnimationData* idleAnimation = AnimationManager::GetInstance()->FindAnimation(animationGroupName_, "Idle")) {
        obj_->SetAnimation(idleAnimation, false);
    }

    if (Model* sizukuModel = ModelManager::GetInstance()->FindModel("pc")) {
        skeleton_ = std::make_unique<Skeleton>(Skeleton().Create(sizukuModel->GetModelData().rootnode));
        skinCluster_ = CreateSkinCluster(*skeleton_, *sizukuModel);
        if (!skinCluster_.mappedPalette.empty()) {
            obj_->SetSkinCluster(&skinCluster_);
        }
    }
}

void PC::Draw()
{
    obj_->Draw();
}


void PC::CheckCollision()
{

    if (OnCollisionRay()) {
        //rayの当たり判定
        if (PlayerCommand::GetInstance()->InteractTrigger()) {
            if (!PlayerCommand::GetIsGrab()) {
                if (animationFinished_) {
                    /*                SEManager::SoundPlay(SEManager::CHAIR);*/
                    if (desiredAnimationName == "Idle") {
                        desiredAnimationName = "Close";
                    } else if (desiredAnimationName == "Close") {
                        desiredAnimationName = "Open";
                    }
                }

            }
        }
    }

}


bool PC::OnCollisionRay()
{
    const std::optional<int32_t> jointIndex = skeleton_->FindJointIndex("monitor");
    skeleton_->SetObjectMatrix(obj_->GetWorldMatrix());
    Vector3 pos = skeleton_->GetJointWorldPosition(skeleton_->GetJoints()[*jointIndex]);
    return playerCamera_->OnCollisionRay(GetAABB(), pos);
}
