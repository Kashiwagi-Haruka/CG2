#include "Door.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"
#include"Animation/AnimationManager.h"
#include"GameObject/SEManager/SEManager.h"
#include"GameObject/Key/Key.h"

bool Door::isSendOpenMessage_ = false;
bool Door::isSendLockMessage_ = false;

Door::Door()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/door", "door");
    obj_->SetModel("door");
    SetAABB({ .min = {-0.125f,0.0f,-0.125f},.max = {0.125f,1.9f,0.125f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer | kCollisionKey);

    autoLockSystem_ = std::make_unique<AutoLockSystem>();

    worldMat_ = Function::MakeIdentity4x4();
    autoLockSystem_->SetParentMat(&worldMat_);

}

Door::~Door()
{


}

void Door::OnCollision(Collider* collider) {

}

Vector3 Door::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(worldMat_);
}

void Door::Update()
{

    isSendLockMessage_ = false;
    isSendOpenMessage_ = false;

    worldMat_ = Function::MakeAffineMatrix(obj_->GetScale(), obj_->GetRotate(), obj_->GetTranslate());
    obj_->SetWorldMatrix(worldMat_);
    obj_->Update();
    Animation();

    if (animationFinished_) {

        if (desiredAnimationName == "1Lock") {
            //アイドル状態に戻す
            desiredAnimationName = "0Idle";
            
        } else  if (desiredAnimationName == "2Open") {

            if (!isOpen_) {
                isOpen_ = true;
            }

            if (autoLockSystem_->GetIsPlayerHit()) {
                desiredAnimationName = "3Close";
            }

        } else if (desiredAnimationName == "3Close") {

            if (isOpen_) {
                isOpen_ = false;
                desiredAnimationName = "0Idle";
            }
        }

    }

    autoLockSystem_->Update();
}

void Door::Initialize()
{
    obj_->Initialize();
    autoLockSystem_->Initialize();

    isSendLockMessage_ = false;
    isSendOpenMessage_ = false;

    isOpen_ = false;

    AnimationManager::GetInstance()->LoadAnimationGroup(animationGroupName_, "Resources/TD3_3102/3d/door", "door");
    AnimationManager::GetInstance()->ResetPlayback(animationGroupName_, "0Idle", true);
    if (const Animation::AnimationData* idleAnimation = AnimationManager::GetInstance()->FindAnimation(animationGroupName_, "Idle")) {
        obj_->SetAnimation(idleAnimation, true);
    }

    if (Model* sizukuModel = ModelManager::GetInstance()->FindModel("door")) {
        skeleton_ = std::make_unique<Skeleton>(Skeleton().Create(sizukuModel->GetModelData().rootnode));
        skinCluster_ = CreateSkinCluster(*skeleton_, *sizukuModel);
        if (!skinCluster_.mappedPalette.empty()) {
            obj_->SetSkinCluster(&skinCluster_);
        }
    }

}

void Door::Draw()
{
    obj_->Draw();
    autoLockSystem_->Draw();
}

void Door::CheckCollision()
{
    //自販機とrayの当たり判定
    if (OnCollisionRay()) {
        if (PlayerCommand::GetInstance()->InteractTrigger()) {
            if (Key::IsGetKey()) {
                desiredAnimationName = "2Open";
                SEManager::SoundPlay(SEManager::DOOR_OPEN);
                isSendOpenMessage_ = true;
            } else {
                desiredAnimationName = "1Lock";
                SEManager::SoundPlay(SEManager::DOOR_LOCK);
                isSendLockMessage_ = true;
            }
        }
    }
}

void Door::Animation()
{



    bool loopAnimation = false;

    const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
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

bool Door::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(GetAABB(), obj_->GetTranslate());
}

void Door::SetPlayerCamera(PlayerCamera* camera)
{
    playerCamera_ = camera;
}

void Door::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();

    autoLockSystem_->SetCamera(camera);
}
