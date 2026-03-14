#include "Door.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"
#include"Animation/AnimationManager.h"

Door::Door()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/door", "door");
    obj_->SetModel("door");
    SetAABB({ .min = {-0.125f,0.0f,-0.125f},.max = {0.125f,1.9f,0.125f} });
    SetCollisionAttribute(kCollisionDoor);
    SetCollisionMask(kCollisionPlayer | kCollisionKey);

    autoLockSystem_ = std::make_unique<AutoLockSystem>();

    worldMat_ = Function::MakeIdentity4x4();
    autoLockSystem_->SetParentMat(&worldMat_);
}

void Door::OnCollision(Collider* collider) {

    if (!isGetKey_) {
        //キーを持っていなかったら
        if (collider->GetCollisionAttribute() == kCollisionKey) {
            isGetKey_ = true;
        }
    }
}


Vector3 Door::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(worldMat_);
}

void Door::Update()
{
    worldMat_ = Function::MakeAffineMatrix(obj_->GetScale(), obj_->GetRotate(), obj_->GetTranslate());
    obj_->SetWorldMatrix(worldMat_);
    obj_->Update();
    Animation();

    if (isGetKey_ && isOpen_) {
        if (autoLockSystem_->GetIsPlayerHit()) {
            desiredAnimationName = "3Close";
        }
    }

    autoLockSystem_->Update();
}

void Door::Initialize()
{
    obj_->Initialize();
    autoLockSystem_->Initialize();

    isGetKey_ = false;
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
            if (isGetKey_) {
                desiredAnimationName = "2Open";
            } else {
                desiredAnimationName = "1Lock";
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

    if (playbackResult.animationFinished) {
        if (desiredAnimationName == "1Lock") {
            desiredAnimationName = "0Idle";
        } else if (desiredAnimationName == "2Open") {
            isOpen_ = true;
        } else if (desiredAnimationName == "3Close") {
            isOpen_ = false;
           /* desiredAnimationName = "0Idle";*/
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
