#include "EdamameModel.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/Player/Player.h"
#include"Object3d/Object3dCommon.h"
#include"Animation/AnimationManager.h"
#include "GameBase.h"
#include"GameObject/YoshidaMath/Easing.h"

#include<algorithm>

EdamameModel::EdamameModel()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/edamame", "edamame");
    obj_->SetModel("edamame");
    SetAABB({ .min = {-0.1f,-0.1f,-0.1f},.max = {0.1f,0.1f,0.1f} });
    SetCollisionAttribute(kCollisionItem);
    SetCollisionMask(kCollisionPlayer | kCollisionFloor);
}


void EdamameModel::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionFloor) {
        if (isDrop_) {
            //落下開始してたら
            desiredAnimationName = "Die";
        }
        speedY_ = 0.0f;
    };
}

Vector3 EdamameModel::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}

void EdamameModel::Animation()
{
    //基本ループしない
    bool loopAnimation = true;


    if (isDrop_) {
        if (desiredAnimationName == "Die") {
            loopAnimation = false;
        }
    } else {
        //アニメーションが終了したら
        if (animationFinished_) {

            //開始且つ落下開始してなかったら
            if (isStart_) {
                //アイドル状態だったら
                if (desiredAnimationName == "Idle") {
                    //回る
                    desiredAnimationName = "Round";
                    loopAnimation = false;
                } else if (desiredAnimationName == "Round") {
                    //話す
                    desiredAnimationName = "Speak";
                }
            }
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

void EdamameModel::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}

void EdamameModel::Update()
{


    if (isDropStart_) {
        if (!isDrop_) {
            rotate_ = Function::DirectionToRotation(YoshidaMath::GetForward(obj_->GetWorldMatrix()), { 1,0,0 });
            rotate_.y += Function::kPi;
            isDrop_ = true;
        }
    }

    Animation();


    //重力処理
    const float deltaTime = GameBase::GetInstance()->GetDeltaTime();


    if (isDrop_) {

        speedY_ -= YoshidaMath::kGravity * deltaTime;
        translate_.y += speedY_ * deltaTime;
        //速度制限
        speedY_ = std::clamp(speedY_, -1.0f, 1.0f);
        //位置制限
        translate_.y = std::clamp(translate_.y, GetAABB().max.y, 2.4f);
    }

    if (isStart_) {
        if (scaleTimer_ != 1.0f) {
            scaleTimer_ += deltaTime;
            scaleTimer_ = std::clamp(scaleTimer_, 0.0f, 1.0f);
            float scale = YoshidaMath::Easing::EaseInOutBack(1.0f, 10.0f, scaleTimer_);
            obj_->SetScale({ scale,scale,scale });
        }
    }

    obj_->SetRotate(rotate_);
    obj_->SetTranslate(translate_);

    if (isDrop_) {
        rotate_.x =  YoshidaMath::Easing::Lerp(rotate_.x,0.0f,0.1f);
        rotate_.z = YoshidaMath::Easing::Lerp(rotate_.z, 0.0f, 0.1f);

        obj_->Update();
    } else {
        obj_->UpdateBillboard();

    }


}

void EdamameModel::Initialize()
{
    isStart_ = false;
    //落下開始
    isDropStart_ = false;
    isDrop_ = false;
    translate_ = { 0.0f };
    scaleTimer_ = 0.0f;

    obj_->Initialize();

    rotate_ = { 0.0f,Function::kPi,0.0f };

    AnimationManager::GetInstance()->LoadAnimationGroup(animationGroupName_, "Resources/TD3_3102/3d/edamame", "edamame");
    AnimationManager::GetInstance()->ResetPlayback(animationGroupName_, "Idle", true);
    if (const Animation::AnimationData* idleAnimation = AnimationManager::GetInstance()->FindAnimation(animationGroupName_, "Idle")) {
        obj_->SetAnimation(idleAnimation, true);
    }

    if (Model* sizukuModel = ModelManager::GetInstance()->FindModel("edamame")) {
        skeleton_ = std::make_unique<Skeleton>(Skeleton().Create(sizukuModel->GetModelData().rootnode));
        skinCluster_ = CreateSkinCluster(*skeleton_, *sizukuModel);
        if (!skinCluster_.mappedPalette.empty()) {
            obj_->SetSkinCluster(&skinCluster_);
        }
    }
}

void EdamameModel::Draw()
{
    obj_->Draw();
}
