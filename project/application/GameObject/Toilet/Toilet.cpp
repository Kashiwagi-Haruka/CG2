#include "Toilet.h"

#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/Player/Player.h"
#include"Object3d/Object3dCommon.h"
#include"Animation/AnimationManager.h"
#include "GameBase.h"
#include"GameObject/SEManager/SEManager.h"

PlayerCamera* Toilet::playerCamera_ = nullptr;

Toilet::Toilet()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/toilet", "toilet");
    obj_->SetModel("toilet");
    // ロッカーより少し低めにAABBを設定 (必要に応じて変更してください)
    SetAABB({ .min = {-0.5f,0.0f,-0.5f},.max = {0.5f,1.2f,0.5f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer | kCollisionKey);
}

void Toilet::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {

    }
}

Vector3 Toilet::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}

void Toilet::Animation()
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

void Toilet::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}

void Toilet::Update()
{
    isOpen_ = false;
    CheckCollision();
    obj_->Update();
    Animation();
}

void Toilet::Initialize()
{
    isRayHit_ = false;
    obj_->Initialize();
    obj_->RegisterEditor(editorRegistrationName_);
    desiredAnimationName = "Close";

    AnimationManager::GetInstance()->LoadAnimationGroup(animationGroupName_, "Resources/TD3_3102/3d/toilet", "toilet");
    AnimationManager::GetInstance()->ResetPlayback(animationGroupName_, desiredAnimationName, false);
    if (const Animation::AnimationData* idleAnimation = AnimationManager::GetInstance()->FindAnimation(animationGroupName_, desiredAnimationName)) {
        obj_->SetAnimation(idleAnimation, false);
    }

    // 変数名をsizukuModelからtoiletModelに変更しています
    if (Model* toiletModel = ModelManager::GetInstance()->FindModel("toilet")) {
        skeleton_ = std::make_unique<Skeleton>(Skeleton().Create(toiletModel->GetModelData().rootnode));
        skinCluster_ = CreateSkinCluster(*skeleton_, *toiletModel);
        if (!skinCluster_.mappedPalette.empty()) {
            obj_->SetSkinCluster(&skinCluster_);
        }
    }
}

void Toilet::Draw()
{
    obj_->Draw();
}

void Toilet::CheckCollision()
{
    isRayHit_ = OnCollisionRay();
    if (isRayHit_) {
        //rayの当たり判定

        if (PlayerCommand::GetInstance()->InteractTrigger()) {

            if (!PlayerCommand::GetIsGrab()) {
                // SEが専用のものがあれば変更してください (例: TOILET_OPEN等)
                SEManager::SoundPlay(SEManager::DOOR_OPEN);
                isOpen_ = !isOpen_;
                if (isOpen_) {
                    desiredAnimationName = "Open";
                } else {
                    desiredAnimationName = "Close";

                }
            }
        }
    }
}

bool Toilet::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(GetAABB(), GetWorldPosition());
}