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
    SetAABB({ .min = {-0.5f,0.0f,-0.5f},.max = {0.5f,1.9f,0.5f} });
    SetCollisionAttribute(kCollisionDoor);
    SetCollisionMask(kCollisionPlayer);
}

void Door::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {

    }
}

Vector3 Door::GetWorldPosition() const
{
    return obj_->GetTranslate();
}

void Door::Update()
{   
    Matrix4x4 worldMat = Function::MakeAffineMatrix(obj_->GetScale(), obj_->GetRotate(), obj_->GetTranslate());
    obj_->SetWorldMatrix(worldMat);
    obj_->Update(); 
    Animation();
}

void Door::Initialize()
{
    obj_->Initialize();

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
}

void Door::CheckCollision()
{
    //自販機とrayの当たり判定
    if (OnCollisionRay()) {
        if (PlayerCommand::GetInstance()->Interact()) {

        }
    }
}

void Door::Animation()
{

    bool loopAnimation = true;
    desiredAnimationName = "2Open";
    if (desiredAnimationName == "0Idle") {
        loopAnimation = false;
    } else if (desiredAnimationName == "1Lock") {
        loopAnimation = false;
    } else if (desiredAnimationName == "2Open") {
        loopAnimation = true;
    }

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
}
