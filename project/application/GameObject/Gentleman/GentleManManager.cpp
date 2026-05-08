#include "GentleManManager.h"
#include"Camera.h"
#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include"Text/GentlemanMenu/GentlemanMenu.h"
#include"GameObject/Player/Player.h"

GentleManManager::GentleManManager()
{
    // セーブポイント紳士
    gentleman_ = std::make_unique<Gentleman>();

}

void GentleManManager::Initialize()
{	// セーブポイント紳士
    gentleman_->Initialize();

}

void GentleManManager::Update()
{

    //最終ステージの時はいない
    if (progressSaveData_->currentStageName == "GentleManStage") {
        return;
    }

    //ミラーステージの時
    if (progressSaveData_->currentStageName == "MirrorStage") {
        gentleman_->SetAnimationName("Idle");
    }

    //チュートリアルステージの時
    if (progressSaveData_->currentStageName == "TutorialStage") {
        gentleman_->SetAnimationName("AerialPigeon");
    }

    // セーブポイント紳士
    gentleman_->Update();

}

void GentleManManager::Draw()
{
    //最終ステージの時はいない
    if (progressSaveData_->currentStageName == "GentleManStage") {
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

void GentleManManager::SetProgressSaveData(ProgressSaveData* progressSaveData)
{
    progressSaveData_ = progressSaveData;

    GentlemanMenu::SetProgressSaveData(progressSaveData_);
}
