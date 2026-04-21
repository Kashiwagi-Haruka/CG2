#include "Elevator.h"
#include "Camera.h"
#include "Function.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include"Animation/AnimationManager.h"
#include"GameObject/SEManager/SEManager.h"
#include "GameBase.h"
#include <cmath>

PlayerCamera* Elevator::playerCamera_ = nullptr;
bool Elevator::isRayHit_ = false;

Elevator::Elevator() {
    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/Elevator", "Elevator");
    modelObj_ = std::make_unique<Object3d>();
    modelObj_->SetModel("Elevator");
    worldMat_ = Function::MakeIdentity4x4();

    for (int i = 0; i < autoLockSystems_.size(); ++i) {
        autoLockSystems_[i] = std::make_unique<AutoLockSystem>();
        autoLockSystems_[i]->SetParentMat(&worldMat_);
    }

    autoLockSystems_[0]->SetTranslate({ 0.0f,0.0f,-4.5f });
    autoLockSystems_[1]->SetAABB({ .min = {-1.5f,0.0f,-1.0f} ,.max = {1.5f,0.02f,2.0f} });
    autoLockSystems_[1]->SetTranslate({ 0.0f,0.0f,0.0f });
}

void Elevator::Initialize() {

    modelObj_->Initialize();
	modelObj_->RegisterEditor("EV");

    elevatorTransform_ = {
        .scale = {1.0f, 1.0f, 1.0f },
        .rotate = {0.0f, Function::kPi, 0.0f },
        .translate = {7.0f, baseHeight_,   -15.0f},
    };

    isRayHit_ = false;
    desiredAnimationName = "Close";
    // 新しい状態管理
    isPlayerInside_ = false;
    insideTimer_ = 0.0f;

    isSceneTransition_ = false;
    isSceneTranstionStart_ = false;

    AnimationManager::GetInstance()->LoadAnimationGroup(animationGroupName_, "Resources/TD3_3102/3d/Elevator", "Elevator");
    AnimationManager::GetInstance()->ResetPlayback(animationGroupName_, desiredAnimationName, false);
    if (const Animation::AnimationData* idleAnimation = AnimationManager::GetInstance()->FindAnimation(animationGroupName_, desiredAnimationName)) {
        modelObj_->SetAnimation(idleAnimation, false);
    }

    if (Model* sizukuModel = ModelManager::GetInstance()->FindModel("Elevator")) {
        skeleton_ = std::make_unique<Skeleton>(Skeleton().Create(sizukuModel->GetModelData().rootnode));
        skinCluster_ = CreateSkinCluster(*skeleton_, *sizukuModel);
        if (!skinCluster_.mappedPalette.empty()) {
            modelObj_->SetSkinCluster(&skinCluster_);
        }
    }


    for (auto& sys : autoLockSystems_) {
        sys->Initialize();
    }

    poster_.SetParentMat(&worldMat_);
    poster_.Initialize();
}

void Elevator::SetCamera(Camera* camera) {
    modelObj_->SetCamera(camera);
    modelObj_->UpdateCameraMatrices();

    for (auto& sys : autoLockSystems_) {
        sys->SetCamera(camera);
    }

    poster_.SetCamera(camera);
}

void Elevator::Update() {

    modelObj_->SetTransform(elevatorTransform_);
    modelObj_->Update();
    worldMat_ = modelObj_->GetWorldMatrix();

    bool hitOuter = autoLockSystems_[0]->IsPlayerHit();
    bool hitInner = autoLockSystems_[1]->IsPlayerHit();

    // 外側マット（入口前）
    if (hitOuter) {
        isPlayerInside_ = false;
        insideTimer_ = 0.0f;

        if (desiredAnimationName == "Close") {
            desiredAnimationName = "Open"; // 外にいる → 開ける
        }
    }

    // 内側マット（エレベーター内部）
    if (hitInner) {
        isPlayerInside_ = true;
        //閉める
        if (desiredAnimationName == "Open") {
            desiredAnimationName = "Close";
        }
    }

    isSceneTranstionStart_ = false;

    if (isPlayerInside_) {

        if (animationFinished_) {

            if (desiredAnimationName == "Close") {

                if (!isSceneTransition_) {
                    isSceneTransition_ = true;
                    isSceneTranstionStart_ = true;
                }


            } else if (desiredAnimationName == "Open") {

                isSceneTransition_ = false;

            }
        }



        insideTimer_ += GameBase::GetInstance()->GetDeltaTime();
        if (insideTimer_ > insideOpenDelay_) {
            if (desiredAnimationName == "Close") {
                desiredAnimationName = "Open";
            }
        }
    }

    // 中にいない & 外側マットも踏んでいない → 扉を閉める
    if (!hitInner && !hitOuter) {
        insideTimer_ = 0.0f;

        if (desiredAnimationName == "Open") {
            desiredAnimationName = "Close";
        }
    }


    Animation();

    for (auto& sys : autoLockSystems_) {
        sys->Update();
    }
    poster_.Update();
}

void Elevator::Draw() {

    modelObj_->Draw();

    for (auto& sys : autoLockSystems_) {
        sys->Draw();
    }

    poster_.Draw();
}

void Elevator::Animation()
{

    bool loopAnimation = false;

    if (desiredAnimationName == "Close") {

    }

    const float deltaTime = GameBase::GetInstance()->GetDeltaTime();
    AnimationManager::PlaybackResult playbackResult{};

    if (AnimationManager::GetInstance()->UpdatePlayback(animationGroupName_, desiredAnimationName, loopAnimation, deltaTime, kAnimationBlendDuration_, blendedPoseAnimation_, playbackResult)) {
        animationFinished_ = playbackResult.animationFinished;

        if (playbackResult.changedAnimation && playbackResult.currentAnimation) {
            modelObj_->SetAnimation(playbackResult.currentAnimation, loopAnimation);
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
