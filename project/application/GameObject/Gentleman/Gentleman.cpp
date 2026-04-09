#include "Gentleman.h"
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
#include"GameSave/GameSave.h"
#include"Text/GentlemanMenu/GentlemanMenu.h"

PlayerCamera* Gentleman::playerCamera_ = nullptr;
bool Gentleman::isRayHit_ = false;

Gentleman::Gentleman()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/gentleman", "gentleman");
    obj_->SetModel("gentleman");
    SetAABB({ .min = {-0.25f, 0.0f, -0.25f}, .max = {0.25f,  1.5f, 0.25f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer);
    localAABB_ = { .min = {-0.5f,-0.5f,-0.5f},.max = {0.5f,0.5,0.5f} };
}

void Gentleman::OnCollision(Collider* collider)
{

}

Vector3 Gentleman::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}

void Gentleman::Animation()
{
    bool loopAnimation = false;

    if (desiredAnimationName == "Idle") {
        loopAnimation = true;
    } else if (desiredAnimationName == "AerialPigeon") {
        loopAnimation = false;
    } else if (desiredAnimationName == "Round") {
        loopAnimation = false;
    } else if (desiredAnimationName == "SitDown") {
        loopAnimation = false;
    } else if (desiredAnimationName == "Sleep") {
        loopAnimation = true;
    } else if (desiredAnimationName == "Soft") {
        loopAnimation = true;
    } else if (desiredAnimationName == "Tired") {
        loopAnimation = false;
    }else if (desiredAnimationName == "Sit") {
        loopAnimation = true;
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



void Gentleman::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}

void Gentleman::Update()
{
    CheckCollision();
    Animation();
    obj_->Update();
}

void Gentleman::Initialize()
{
    obj_->Initialize();
    animationNum = 0;

    isRayHit_ = false;
    isPreOnCollisionRay_ = false;
    desiredAnimationName = "Idle";

    AnimationManager::GetInstance()->LoadAnimationGroup(animationGroupName_, "Resources/TD3_3102/3d/gentleman", "gentleman");
    AnimationManager::GetInstance()->ResetPlayback(animationGroupName_, desiredAnimationName, false);
    if (const Animation::AnimationData* idleAnimation = AnimationManager::GetInstance()->FindAnimation(animationGroupName_, desiredAnimationName)) {
        obj_->SetAnimation(idleAnimation, false);
    }

    if (Model* sizukuModel = ModelManager::GetInstance()->FindModel("gentleman")) {
        skeleton_ = std::make_unique<Skeleton>(Skeleton().Create(sizukuModel->GetModelData().rootnode));
        skinCluster_ = CreateSkinCluster(*skeleton_, *sizukuModel);
        if (!skinCluster_.mappedPalette.empty()) {
            obj_->SetSkinCluster(&skinCluster_);
        }
    }
}

void Gentleman::Draw()
{
    obj_->Draw();
}


void Gentleman::CheckCollision()
{

    isRayHit_ = OnCollisionRay();

    // Rayが外れたらメニューを自動で閉じる
    if (isPreOnCollisionRay_ && !isRayHit_ && GentlemanMenu::GetIsShowMenu()) {
        GentlemanMenu::SetIsShowMenu(false);
    }

    if (PlayerCommand::GetInstance()->MouseWheelUp()) {

        if (animationNum < 7) {
            animationNum++;
        } else {
            animationNum = 0;
        }
    }

    std::string animationName = "Idle";

    if (animationNum == 0) {
        animationName = "Idle";
    } else if (animationNum == 1) {
        animationName = "AerialPigeon";
    } else if (animationNum == 2) {
        animationName = "Round";
    } else if (animationNum == 3) {
        animationName = "SitDown";
    } else if (animationNum == 4) {
        animationName = "Sleep";
    } else if (animationNum == 5) {
        animationName = "Soft";
    } else if (animationNum == 6) {
        animationName = "Tired";
    } else if (animationNum == 7) {
        animationName = "Sit";
    }

    //rayの当たり判定
    if (isRayHit_ && PlayerCommand::GetInstance()->InteractTrigger() && !PlayerCommand::GetIsGrab()) {
        //トリガーしたとき且つ何も持ってないとき

        if (GentlemanMenu::GetIsShowMenu()) {
            // 数値によって処理を変更する
            SwichCommand();
        } else {
            // メニューを表示してないとき表示する
            GentlemanMenu::SetIsShowMenu(true);
        }

        SEManager::SoundPlay(SEManager::TYPE);
        SetAnimationName(animationName);

    }

    isPreOnCollisionRay_ = isRayHit_;

#ifdef USE_IMGUI
    ImGui::Begin("Gentleman");
    ImGui::Text("animationNum : %d", animationNum);
    ImGui::Text("CurrantAnimation : %s", desiredAnimationName.c_str());
    ImGui::End();

#endif
}


bool Gentleman::OnCollisionRay()
{
    const std::optional<int32_t> jointIndex = skeleton_->FindJointIndex("Root_Upper");
    skeleton_->SetObjectMatrix(obj_->GetWorldMatrix());
    Vector3 pos = skeleton_->GetJointWorldPosition(skeleton_->GetJoints()[*jointIndex]);
    return playerCamera_->OnCollisionRay(localAABB_, pos);
}
void Gentleman::SwichCommand()
{
    switch (GentlemanMenu::GetSelectButtonNum())
    {
    case GentlemanMenu::TALK:
        //メニューを表示しているとき


        //グラブ終了後メニューを閉じる
        //GentlemanMenu::SetIsShowMenu(false);

        break;
    case GentlemanMenu::SAVE:

        if (!GentlemanMenu::GetIsSaveMenuShow()) {
            GentlemanMenu::SetIsSaveMenuShow(true);
        }

        //メニューを閉じる
        GentlemanMenu::SetIsShowMenu(false);
        break;
    default:
        //メニューを閉じる
        GentlemanMenu::SetIsShowMenu(false);
        break;
    }



}