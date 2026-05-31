#include "WalkWhiteBoard.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"Object3d/Object3dCommon.h"
#include "Model/ModelManager.h"
#include"DirectXCommon.h"
#include<imgui.h>
#include"Animation/AnimationManager.h"
#include"GameBase.h"
#include"GameObject/Key/Key.h"
#include"GameObject/SEManager/SEManager.h"

WalkWhiteBoard::WalkWhiteBoard()
{
    localAABB_ = { .min = { -0.5f,-0.5f,-0.5f},.max = {0.5f,0.5f,0.5f} };
    /* SetRadius(1.0f);*/
    SetAABB(AABB{ .min = {-0.0f,0.5f,-0.5f}, .max = {0.5f,1.5f,0.5f} });
    SetCollisionAttribute(kCollisionEnemy);
    SetCollisionMask(kCollisionPlayer | kCollisionWall);
}

void WalkWhiteBoard::OnCollision(Collider* collider)
{

    if (collider->GetCollisionAttribute() == kCollisionPlayer) {

    }


}

void WalkWhiteBoard::Initialize()
{
    isMove_ = false;
    obj_->Initialize();
    obj_->RegisterEditor(editorRegistrationName_);
    transform_ = { .scale = {1.0f,1.0f,1.0f}, .rotate = {0.0f,Function::kPi ,0.0f},.translate = {0.0f,0.0f,0.0f} };
    velocity_ = { 0.0f };

#ifdef _DEBUG
    primitive_->Initialize(Primitive::Box);
    primitive_->SetColor({ 1.0f,1.0f,1.0f,0.1f });
#endif

    AnimationManager::GetInstance()->LoadAnimationGroup(editorRegistrationName_, "Resources/TD3_3102/3d/whiteBoard", "whiteBoard");
    AnimationManager::GetInstance()->ResetPlayback(editorRegistrationName_, "Idle", true);
    if (const Animation::AnimationData* idleAnimation = AnimationManager::GetInstance()->FindAnimation(editorRegistrationName_, "Idle")) {
        obj_->SetAnimation(idleAnimation, true);
    }

    if (Model* walkModel = ModelManager::GetInstance()->FindModel("whiteBoard")) {
        skeleton_ = std::make_unique<Skeleton>(Skeleton().Create(walkModel->GetModelData().rootnode));
        skinCluster_ = CreateSkinCluster(*skeleton_, *walkModel);
        if (!skinCluster_.mappedPalette.empty()) {
            obj_->SetSkinCluster(&skinCluster_);
        }
    }


}

void WalkWhiteBoard::Update()
{

    transform_ = obj_->GetTransform();

    if (!isMove_ && Key::GetGetKeyMessage()) {
        //鍵を取得すると動き出すよ～
        isMove_ = true;
        if (SEManager::IsSoundFinished(SEManager::MYSTERY)) {
            SEManager::SoundPlay(SEManager::MYSTERY);
        }
       
        desiredAnimationName = "WalkStart";
    }

    if (isMove_) {
        if (targetPos_) {

            YoshidaMath::LookTarget(*targetPos_, transform_);
            velocity_ = YoshidaMath::GetToTargetVec(*targetPos_, transform_.translate);
            const float deltaTime = GameBase::GetInstance()->GetDeltaTime();
            velocity_ *= deltaTime;
            transform_.translate.x += velocity_.x;
            transform_.translate.z += velocity_.z;
        }
    }

    obj_->SetTransform(transform_);
    obj_->Update();

    Animation();
    Vector3 translate = { 0.0f,1.375f,0.0f };
    Matrix4x4 child = Function::MakeTranslateMatrix(translate);
    child =   Function::Multiply(child,obj_->GetWorldMatrix());
   
    collisionTransform_.translate = YoshidaMath::GetWorldPosByMat(child);
    collisionTransform_.rotate = transform_.rotate;
    collisionTransform_.scale = YoshidaMath::GetAABBScale(localAABB_);

#ifdef _DEBUG
    primitive_->SetTransform(collisionTransform_);
    primitive_->Update();
#endif
#ifdef USE_IMGUI
    ImGui::Begin("WalkWhiteBoard");
    ImGui::Checkbox("isMove", &isMove_);
    ImGui::End();
#endif
}

void WalkWhiteBoard::Draw()
{
    Object3dCommon::GetInstance()->DrawCommonSkinning();
    obj_->Draw();

#ifdef _DEBUG
    Object3dCommon::GetInstance()->DrawCommon();
    primitive_->Draw();
#endif
}

void WalkWhiteBoard::ResetCollisionAttribute()
{
    SetCollisionAttribute(kCollisionEnemy);
}

void WalkWhiteBoard::SetCollisionAttributeNoneAndInitialize()
{
    desiredAnimationName = "Idle";
    SetCollisionAttribute(kCollisionNone);
    isMove_ = false;
}

void WalkWhiteBoard::Animation()
{
    bool loopAnimation = false;
    if (desiredAnimationName == "Idle") {
        loopAnimation = false;
    } else if (desiredAnimationName == "WalkStart") {
        loopAnimation = false;
    } else if (desiredAnimationName == "Walking") {
        loopAnimation = true;
    }


    const float deltaTime = GameBase::GetInstance()->GetDeltaTime();
    AnimationManager::PlaybackResult playbackResult{};
    if (AnimationManager::GetInstance()->UpdatePlayback(editorRegistrationName_, desiredAnimationName, loopAnimation, deltaTime, kAnimationBlendDuration_, blendedPoseAnimation_, playbackResult)) {
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

    if (animationFinished_) {

        if (desiredAnimationName == "WalkStart") {
            desiredAnimationName = "Walking";
        }
    }


}
