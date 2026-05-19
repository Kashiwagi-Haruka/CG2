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
#include<imgui.h>
PlayerCamera* Locker::playerCamera_ = nullptr;

Locker::Locker()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/Locker", "Locker");
    obj_->SetModel("Locker");
    SetAABB({ .min = {-0.25f,0.0f,-0.25f},.max = {0.25f,1.8f,0.25f} });
    SetCollisionAttribute(kCollisionLocker);
    SetCollisionMask(kCollisionPlayer | kCollisionKey);
    worldMatrix_ = Function::MakeIdentity4x4();
}

void Locker::OnCollision(Collider* collider)
{

    if (collider->GetCollisionAttribute() == kCollisionPlayer) {
        //プレイヤーがロッカーの中に入ったらドアを閉める

        if (isPlayerIn_) {
            return;
        }

       isPlayerIn_ = true;

    
    }
}

Vector3 Locker::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(worldMatrix_);
}

void Locker::Animation()
{

    bool loopAnimation = false;

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

    for (auto& [name, collider] : colliders_) {
        collider->SetCamera(camera);
    }
}

void Locker::Update()
{

    CheckCollision();
    obj_->Update();
    worldMatrix_ = obj_->GetWorldMatrix();

    //当たり判定
    for (auto& [name, collider] : colliders_) {
        collider->Update();
    }

    Animation();

#ifdef USE_IMGUI
    ImGui::Begin(editorRegistrationName_.c_str());
    ImGui::Checkbox("Open", &isOpen_);
    ImGui::Checkbox("isPlayerIn", &isPlayerIn_);
    ImGui::Checkbox("isPlayerPreIn_", &isPlayerPreIn_);



    ImGui::End();
#endif
}

void Locker::Initialize()
{
    isRayHit_ = false;
    isOpen_ = false;
    isPlayerIn_ = false;
    isPlayerPreIn_ = false;
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

    colliders_.clear();

    std::string back = editorRegistrationName_ + "_Back";
    std::string left = editorRegistrationName_ + "_Left";
    std::string right = editorRegistrationName_ + "_Right";
    std::string front = editorRegistrationName_ + "_Front";

    colliders_[back] = std::make_unique<ObjectCollider>();
    colliders_[left] = std::make_unique<ObjectCollider>();
    colliders_[right] = std::make_unique<ObjectCollider>();
    colliders_[front] = std::make_unique<ObjectCollider>();

    for (auto& [name, collider] : colliders_) {
        collider->SetParentMatrix(&worldMatrix_);
        collider->Initialize(YoshidaMath::ColliderType::kAABB);
        collider->RegisterEditor(name);
    }

}

void Locker::Draw()
{

    //ロッカーに入ったら描画されない
    if (!isPlayerIn_) {
        obj_->Draw();
    }



    //for (auto& [name, collider] : colliders_) {
    //    collider->Draw();
    //}
}


void Locker::CheckCollision()
{
    isRayHit_ = OnCollisionRay();

   

        //rayの当たり判定

            if (PlayerCommand::GetInstance()->InteractTrigger()&&
                !PlayerCommand::GetIsGrab()&&
                isRayHit_|| !PlayerCommand::GetIsGrab() && !isPlayerPreIn_&&isPlayerIn_) {

                SEManager::SoundPlay(SEManager::LOCKER);
               

                if (desiredAnimationName == "Open") {
                    desiredAnimationName = "Close";
                    isOpen_ = false;
                } else {

                    if (desiredAnimationName == "Close"|| desiredAnimationName == "Idle") {
                        desiredAnimationName = "Open";
                        isOpen_ = true;
                    }
                }
  
         

            }
        
            isPlayerPreIn_ = isPlayerIn_;


}


bool Locker::OnCollisionRay()
{
    //ロッカーの前に例を飛ばす
    std::string front = editorRegistrationName_ + "_Front";
    return playerCamera_->OnCollisionRay(GetAABB(), colliders_[front]->GetWorldPosition());
}
