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
    SetAABB({ .min = {-0.5f, 0.0f, -0.5f}, .max = {0.5f,  1.5f, 0.5f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer);
    localAABB_ = { .min = {-0.5f,-0.5f,-0.5f},.max = {0.5f,0.5,0.5f} };
    //紳士トーク
    gentlemanTalk_ = std::make_unique<GentlemanTalk>();
    handMat_ = Function::MakeIdentity4x4();
}

void Gentleman::OnCollision(Collider* collider)

{

}

Vector3 Gentleman::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}

Matrix4x4 Gentleman::GetJointMatrix(const char* jointName) const
{
    if (!skeleton_) {
        return { 0.0f };
    }

    const std::optional<int32_t> jointIndex = skeleton_->FindJointIndex(jointName);
    if (!jointIndex.has_value()) {
        return { 0.0f };
    }

    skeleton_->SetObjectMatrix(obj_->GetWorldMatrix());

    return  skeleton_->GetJointWorldMatrix(skeleton_->GetJoints()[*jointIndex]);
}

Matrix4x4& Gentleman::GetHandMat()
{

    return  handMat_;
    // TODO: return ステートメントをここに挿入します
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
    } else if (desiredAnimationName == "Sleep") {
        loopAnimation = true;
    } else if (desiredAnimationName == "SleepStand") {
        loopAnimation = true;
    } else if (desiredAnimationName == "SleepStandPortal") {
        loopAnimation = true;
    } else if (desiredAnimationName == "Soft") {
        loopAnimation = true;
    } else if (desiredAnimationName == "Tired") {
        loopAnimation = false;
    } else if (desiredAnimationName == "Sit") {
        loopAnimation = true;
    } else if (desiredAnimationName == "SitDown") {
        loopAnimation = false;
    } else if (desiredAnimationName == "SitTalk") {
        loopAnimation = true;
    } else if (desiredAnimationName == "Talk") {
        loopAnimation = true;
    }

    if (animationFinished_) {
        if (desiredAnimationName == "Tired") {
            //倒れたらスリープ
            desiredAnimationName = "Sleep";
        }

        if (desiredAnimationName == "SitDown") {
            desiredAnimationName = "Sit";

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



void Gentleman::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}

void Gentleman::Update()
{
    //紳士トーク更新 毎フレームメッセージをリセット中
    gentlemanTalk_->Update();

    CheckCollision();
    Animation();
    obj_->Update();
    handMat_ = GetJointMatrix("Hand.R");
}

void Gentleman::Initialize()
{
    obj_->Initialize();
    obj_->RegisterEditor("gentleman");
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

    //紳士トーク
    gentlemanTalk_->Initialize();



}

void Gentleman::Draw()
{
    Object3dCommon::GetInstance()->DrawCommonSkinning();
    obj_->Draw();
}


void Gentleman::CheckCollision()
{

    isRayHit_ = OnCollisionRay();

    // Rayが外れたらメニューを自動で閉じる
    if (isPreOnCollisionRay_ && !isRayHit_ && GentlemanMenu::GetIsShowMenu()) {
        GentlemanMenu::SetIsShowMenu(false);
    }

    if (!GentlemanMenu::GetIsShowMenu()) {
        //メニューを表示していないときトークも表示しない
        gentlemanTalk_->SetIsDraw(false);
    }

    //rayの当たり判定 疲れて寝てないときなど
    if (isRayHit_ &&
        PlayerCommand::GetInstance()->InteractTrigger() &&
        !PlayerCommand::GetIsGrab()
        && desiredAnimationName != "Sleep"
        && desiredAnimationName != "Tired"
        ) {
        //トリガーしたとき且つ何も持ってないとき

        if (GentlemanMenu::GetIsShowMenu()) {
            // 数値によって処理を変更する
            SwichCommand();
        } else {
            // メニューを表示してないとき表示する
            GentlemanMenu::SetIsShowMenu(true);
        }

        SEManager::SoundPlay(SEManager::TYPE);

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
    //舌のコントロールボーン
    const std::optional<int32_t> jointIndex = skeleton_->FindJointIndex("CON.tongue.002");
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
       //gentlemanTalk_->SetStrings(progressSaveData_->currentStageName);
        //gentlemanTalk_->SetIsDraw(true);
        gentlemanTalk_->GentlemanSendMessage();

        break;
    case GentlemanMenu::SAVE:

        if (!GentlemanMenu::GetIsSaveMenuShow()) {
            GentlemanMenu::SetIsSaveMenuShow(true);
        }

        //紳士トークが描画されていたら描画を外す
        gentlemanTalk_->SetIsDraw(false);
        //メニューを閉じる
        GentlemanMenu::SetIsShowMenu(false);
        break;
    default:

        //メニューを閉じる
        GentlemanMenu::SetIsShowMenu(false);
        break;
    }



}