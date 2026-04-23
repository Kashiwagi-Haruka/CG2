#include "Locker.h"

#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/Player/Player.h"
#include"Object3d/Object3dCommon.h"
#include"Animation/AnimationManager.h"
#include "GameBase.h"
#include"GameObject/SEManager/SEManager.h"
PlayerCamera* Locker::playerCamera_ = nullptr;

Locker::Locker()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/Locker", "Locker");
    obj_->SetModel("Locker");
    SetAABB({ .min = {-0.5f,0.0f,-0.5f},.max = {0.5f,1.8f,0.5f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer| kCollisionKey);
}

void Locker::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {

    }
}

Vector3 Locker::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}

void Locker::Animation()
{

    bool loopAnimation = false;

    if (desiredAnimationName == "Close") {
        if (animationFinished_) {
            //アニメーションの終了後アイドル状態にする
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

void Locker::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}

void Locker::Update()
{
    isOpen_ = false;
    CheckCollision();
    obj_->Update();
    Animation();
}

void Locker::Initialize()
{
    isRayHit_ = false;
    obj_->Initialize();
	obj_->RegisterEditor(editorRegistrationName_);
    desiredAnimationName = "Idle";

    AnimationManager::GetInstance()->LoadAnimationGroup(animationGroupName_, "Resources/TD3_3102/3d/Locker", "Locker");
    AnimationManager::GetInstance()->ResetPlayback(animationGroupName_, desiredAnimationName, false);
    if (const Animation::AnimationData* idleAnimation = AnimationManager::GetInstance()->FindAnimation(animationGroupName_, desiredAnimationName)) {
        obj_->SetAnimation(idleAnimation, false);
    }

    if (Model* sizukuModel = ModelManager::GetInstance()->FindModel("Locker")) {
        skeleton_ = std::make_unique<Skeleton>(Skeleton().Create(sizukuModel->GetModelData().rootnode));
        skinCluster_ = CreateSkinCluster(*skeleton_, *sizukuModel);
        if (!skinCluster_.mappedPalette.empty()) {
            obj_->SetSkinCluster(&skinCluster_);
        }
    }

}

void Locker::Draw()
{
    obj_->Draw();
}


void Locker::CheckCollision()
{
    isRayHit_ = OnCollisionRay();
    if (isRayHit_) {
        //rayの当たり判定

        if (PlayerCommand::GetInstance()->InteractTrigger()) {
       
            if (!PlayerCommand::GetIsGrab()) {
                SEManager::SoundPlay(SEManager::DOOR_OPEN);
                isOpen_ = !isOpen_;
                if (isOpen_) {
                    if (desiredAnimationName == "Idle") {
                        desiredAnimationName = "Open";
                    } 
                } else {
                    if (desiredAnimationName == "Open") {
                        desiredAnimationName = "Close";
                    }
                }
           

            }
        }
    }
 
}


bool Locker::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(GetAABB(), GetWorldPosition());
}
