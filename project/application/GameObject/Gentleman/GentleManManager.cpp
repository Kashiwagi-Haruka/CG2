#include "GentleManManager.h"
#include"Camera.h"
#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include"Text/GentlemanMenu/GentlemanMenu.h"
#include"GameObject/Player/Player.h"
#include"GentlemanTalk.h"
#include"GameObject/Key/Key.h"
#include"GameObject/SEManager/SEManager.h"

GentleManManager::GentleManManager()
{
    // セーブポイント紳士
    gentleman_ = std::make_unique<Gentleman>();

}

void GentleManManager::Initialize()
{	// セーブポイント紳士
    gentleman_->Initialize();
    isTired_ = false;
    isSit_ = false;
}

void GentleManManager::Update()
{


   auto saveData = GameSave::GetInstance().GetProgressSaveData();

    //最終ステージの時はいない
    if (saveData.currentStageName == "GentleManStage") {
        return;
    }

    //チュートリアルステージの時
    if (saveData.currentStageName == "TutorialStage") {

        if (GentlemanTalk::GetIsTalkEnd()) {
            gentleman_->SetAnimationName("Idle");
        } else {
            gentleman_->SetAnimationName("Talk");
        }

    }

    //ミラーステージの時
    if (saveData.currentStageName == "MirrorStage") {

        if (Gentleman::IsRayHit()) {


            if (isSit_) {
                if (GentlemanTalk::GetIsTalkEnd()) {
                    gentleman_->SetAnimationName("Sit");
                } else {
                    gentleman_->SetAnimationName("SitTalk");
                }
            } else {


                gentleman_->SetAnimationName("SitDown");
                isSit_ = true;



            }



        }



    }

    //落下の時
    if (saveData.currentStageName == "ElevatorFallStage") {
        gentleman_->SetAnimationName("AerialPigeon");
    }

    //落下の時
    if (saveData.currentStageName == "RestroomStage") {
        if (Key::IsGetKey()) {

            gentleman_->SetAnimationName("Soft");

        } else {
            //  rayがヒットすると...

            if (Gentleman::IsRayHit()) {

                if (!isTired_) {
                    gentleman_->SetAnimationName("Tired");
                    isTired_ = true;

                    //ドサッと倒れる音
                    SEManager::SoundPlay(SEManager::FALL_DOWN);
                }

            }

            if (!isTired_) {
                gentleman_->SetAnimationName("Idle");
            }

        }

    }

    // セーブポイント紳士
    gentleman_->Update();

}

void GentleManManager::Draw()
{

    auto saveData = GameSave::GetInstance().GetProgressSaveData();

    //最終ステージの時はいない
    if (saveData.currentStageName == "GentleManStage") {
        return;
    }

    // セーブポイント紳士
    gentleman_->Draw();


}

void GentleManManager::SetCamera(Camera* camera)
{
    // セーブポイント紳士
    gentleman_->SetCamera(camera);

}

void GentleManManager::SetPlayerCamera(PlayerCamera* playerCamera)
{
    // セーブポイント紳士
    gentleman_->SetPlayerCamera(playerCamera);

    //紳士メニュー
    GentlemanMenu::SetPlayerCamera(playerCamera);
}

void GentleManManager::SetPlayer(Player* player)
{
    player_ = player;
    GentlemanMenu::SetPlayerTransform(&player_->GetTransform());
}

