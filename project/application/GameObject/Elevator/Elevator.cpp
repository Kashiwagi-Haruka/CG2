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

namespace {
    const int kMaxWall = 3;
}

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
    autoLockSystems_[0]->SetAABB({ .min = {-1.5f,0.0f,-0.5f} ,.max = {1.5f,0.02f,0.5f} });

    autoLockSystems_[1]->SetAABB({ .min = {-1.5f,0.0f,-1.0f} ,.max = {1.5f,0.02f,2.0f} });
    autoLockSystems_[1]->SetTranslate({ 0.0f,0.0f,0.0f });



    for (int i = 0; i < kMaxWall; ++i) {
        std::unique_ptr<Wall> wall = std::make_unique<Wall>();
        wall->SetParentMatrix(&worldMat_);
        walls_.push_back(std::move(wall));
    }

}

Elevator::~Elevator()
{
    for (auto& wall : walls_) {
        if (wall != nullptr) {
            wall.reset();
            wall = nullptr;
        }
    }

    walls_.clear();
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

    // 壁の初期化
    for (auto& wall : walls_) {
        wall->Initialize();
    }
}

void Elevator::SetCamera(Camera* camera) {
    modelObj_->SetCamera(camera);
    modelObj_->UpdateCameraMatrices();

    for (auto& sys : autoLockSystems_) {
        sys->SetCamera(camera);
    }

    poster_.SetCamera(camera);

    for (auto& wall : walls_) {
        wall->SetCamera(camera);
    }
}

void Elevator::Update() {

    modelObj_->SetTransform(elevatorTransform_);
    modelObj_->Update();

    worldMat_ = modelObj_->GetWorldMatrix();

    //常にシーンの切り替わる瞬間をリセットする
    isSceneTranstionStart_ = false;

    CheckCollision();

    Inside();

    Animation();

    for (auto& sys : autoLockSystems_) {
        sys->Update();
    }
    //エレベーター内のポスターの更新
    poster_.Update();

    walls_[0]->SetST({ 2.0f,4.0f,4.0f }, { -2.0f ,0.0f,0.0f });
    walls_[1]->SetST({ 2.0f,4.0f,4.0f }, { 2.0f  ,0.0f,0.0f });
    walls_[2]->SetST({ 4.0f,4.0f,2.0f }, { 0.0f,0.0f, 2.0f });

    for (auto& wall : walls_) {
        wall->Update();
    }
}

void Elevator::Draw() {

    modelObj_->Draw();

    for (auto& sys : autoLockSystems_) {
        sys->Draw();
    }

    for (auto& wall : walls_) {
        wall->Draw();
    }

    poster_.Draw();
}

void Elevator::Close() {
    //開いているとき閉める
    if (desiredAnimationName == "Open") {
        SEManager::SoundPlay(SEManager::ELEVATOR_OPEN);
        desiredAnimationName = "Close";
    }
}

void Elevator::Open() {
    //しまっているとき開く
    if (desiredAnimationName == "Close") {
        SEManager::SoundPlay(SEManager::ELEVATOR_OPEN);
        desiredAnimationName = "Open"; // 外にいる → 開ける
    }
}

void Elevator::CheckCollision() {


    Vector3 distance = Function::Distance(playerCamera_->GetTransform().translate, modelObj_->GetTranslate());
    float length = Function::Length(distance);
    // 距離が長いとき
    if (length >= 10.0f) {
        insideTimer_ = 0.0f;
        isPlayerInside_ = false;
        if (desiredAnimationName == "Open") {
            desiredAnimationName = "Close";
        }
    } else {

        //当たり判定を取得しておく
        bool hitOuter = autoLockSystems_[0]->IsPlayerHit();
        bool hitInner = autoLockSystems_[1]->IsPlayerHit();

        // 外側マット（入口前）
        if (hitOuter) {
            //プレイヤーは外側にいる
            isPlayerInside_ = false;
            //内側にいるフラグをリセット
            insideTimer_ = 0.0f;

            if (!autoLockSystems_[0]->IsPlayerPreHit()) {
                Open();
            }
        }

        // 内側マット（エレベーター内部）
        if (hitInner) {
            //プレイヤーは内側にいる
            isPlayerInside_ = true;

            if (!autoLockSystems_[1]->IsPlayerPreHit()) {
                Close();
            }


        }

    }

}

void Elevator::Animation()
{

    bool loopAnimation = false;
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

void Elevator::Inside() {

    //プレイヤーが内側にいる時の処理
    if (!isPlayerInside_) {
        return;
    }

    if (insideTimer_ < insideOpenDelay_) {
        //タイマーを回す
        insideTimer_ += GameBase::GetInstance()->GetDeltaTime();
        insideTimer_ = std::clamp(insideTimer_, 0.0f, insideOpenDelay_);
        if (insideTimer_ == insideOpenDelay_) {
            //タイマーが一定時間になった時開ける
            if (desiredAnimationName == "Close") {
                SEManager::SoundPlay(SEManager::ELEVATOR_OPEN);
                SEManager::SoundPlay(SEManager::ELEVATOR_BELL);
                desiredAnimationName = "Open";
                isSceneTransition_ = false;
            }
        }
    }

    //アニメーションの終了を感知した時
    if (animationFinished_) {

        if (desiredAnimationName == "Close") {
            //しまっていたとき
            if (!isSceneTransition_) {
                //シーンの切り替わる瞬間を取得する
                isSceneTransition_ = true;
                isSceneTranstionStart_ = true;
            }
        }
    }

}
