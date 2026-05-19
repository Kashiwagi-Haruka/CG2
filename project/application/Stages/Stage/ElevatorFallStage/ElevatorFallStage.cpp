#include "ElevatorFallStage.h"
#include<assert.h>

#include "GameObject/Player/Player.h"
#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include "Engine/Editor/EditorTool/Hierarchy/Hierarchy.h"
#include"Function.h"
#include "GameObject/Flashlight/Flashlight.h"
#include "GameObject/WhiteBoard/WhiteBoardManager.h"
#include"GameObject/TimeCard/TimeCardWatch.h"
#include "GameObject/Portal/PortalManager.h"

void ElevatorFallStage::InitializeLights()
{
    assert(lightManager_);
    lightManager_->ClearLights();
    lightManager_->Initialize();
    lightManager_->SetActiveLightCount(Yoshida::LightManager::POINT, 3);

}

void ElevatorFallStage::UpdateLights()
{
}

ElevatorFallStage::ElevatorFallStage(Player* player)
{
    player_ = player;


    whiteBoardManager_ = std::make_unique<WhiteBoardManager>(&player_->GetTransform().translate, 10, 0);
    portalManager_ = std::make_unique<PortalManager>(&player_->GetTransform().translate);

    portalManager_->SetWhiteBoardManager(whiteBoardManager_.get());

    timeCardWatch_ = std::make_unique<TimeCardWatch>();
    timeCardWatch_->SetPlayer(player_);

    //懐中電灯の作成
    flashlight_ = std::make_unique<Flashlight>();
    flashlight_->SetPlayer(player_);

}

void ElevatorFallStage::Initialize()
{
    Hierarchy* hierarchy = Hierarchy::GetInstance();
    hierarchy->BeginRegisterFile("ElevatorFallStage_objectEditors.json");

    InitializeLights();
    portalManager_->Initialize();
    whiteBoardManager_->Initialize();
    timeCardWatch_->Initialize();

    //懐中電灯の初期化
    flashlight_->Initialize();

    hierarchy->LoadObjectEditorsFromJsonIfExists("ElevatorFallStage_objectEditors.json");
    hierarchy->EndRegisterFile();
}

void ElevatorFallStage::SetPlayer(Player* player)
{
    player_ = player;
}

void ElevatorFallStage::SetCollisionManager(CollisionManager* collisionManager)
{
    stageCollisionManager_ = collisionManager;
}

void ElevatorFallStage::UpdateGameObject(Camera* camera, const Vector3& lightDirection, Player* player)
{
    portalManager_->WarpPlayer(player);

    Vector3 translate = player->GetWorldPosition();
    if (translate.y <= -20.0f) {

        translate = whiteBoardManager_->GetWhiteBoards().at(0)->GetWorldPosition();
        translate.y = -5.0f;
        player-> ResetVelocityY(0.0f);
        player->SetTranslate(translate);
    }
    whiteBoardManager_->Update();
    //懐中電灯の更新
    flashlight_->Update();

    UpdateLights();
}

void ElevatorFallStage::UpdatePortal()
{
    portalManager_->Update();
}

void ElevatorFallStage::CheckCollision()
{
    if (!stageCollisionManager_) {
        return;
    }

    portalManager_->CheckCollision();

    for (auto& portal : portalManager_->GetPortals()) {
        if (!portal->GetIsPlayerCanWarp()) {
            stageCollisionManager_->AddCollider(portal.get());
        } else {
            break;
        }
    }


    for (auto& whiteBoard : whiteBoardManager_->GetWhiteBoards()) {
        stageCollisionManager_->AddCollider(whiteBoard.get());
    }

    stageCollisionManager_->AddCollider(flashlight_.get());
    stageCollisionManager_->CheckAllCollisions();
}

void ElevatorFallStage::DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle)
{

    timeCardWatch_->Draw();
    whiteBoardManager_->Draw();
    flashlight_->Draw();
    portalManager_->Draw(isShadow, drawPortal, isDrawParticle);
}

void ElevatorFallStage::DrawSprite()
{
}

void ElevatorFallStage::SetSceneCameraForDraw(Camera* camera)
{
    portalManager_->SetCamera(camera);
    timeCardWatch_->SetCamera(camera);
    flashlight_->SetCamera(camera);
    whiteBoardManager_->SetCamera(camera);
}

void ElevatorFallStage::SetPlayerCamera(PlayerCamera* playerCamera)
{
    portalManager_->SetPlayerCamera(playerCamera);
    flashlight_->SetPlayerCamera(playerCamera);
}

PortalManager* ElevatorFallStage::GetPortalManager()
{
    return portalManager_.get();
}

void ElevatorFallStage::SetLightManager(Yoshida::LightManager* lightManager)
{
    lightManager_ = lightManager;
}
