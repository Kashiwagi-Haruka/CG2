#include "ElevatorFallStage.h"
#include<assert.h>

#include "GameObject/Player/Player.h"
#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include "Engine/Editor/EditorTool/Hierarchy/Hierarchy.h"
#include"Function.h"
#include "GameObject/Flashlight/Flashlight.h"
#include "GameObject/WhiteBoard/WhiteBoardManager.h"
#include"GameObject/TimeCard/TimeCardWatch.h"
#include "GameObject/Portal/FallPortalManager.h"
#include "GameObject/Wall/WallManager2.h"
#include "GameObject/TestField/TestField.h"
#include "GameObject/Door/Door.h"

#include "GameObject/Key/Key.h"

void ElevatorFallStage::InitializeLights()
{
    assert(lightManager_);
    lightManager_->ClearLights();
    lightManager_->Initialize();
    lightManager_->SetActiveLightCount(Yoshida::LightManager::POINT, 3);
    lightManager_->SetActiveLightCount(Yoshida::LightManager::AREA, 2);

    lightManager_->SetAreaLight(wallManager2_->GetAreaLights().at(0), 0);
    lightManager_->SetAreaLight(wallManager2_->GetAreaLights().at(1), 1);

    lightManager_->SetActiveLightCount(Yoshida::LightManager::SPOT, 1);
    lightManager_->SetSpotLight(flashlight_->GetSpotLight(), 0);
}

void ElevatorFallStage::UpdateLights()
{
    lightManager_->SetAreaLight(wallManager2_->GetAreaLights().at(0), 2);
    lightManager_->SetAreaLight(wallManager2_->GetAreaLights().at(1), 3);

    lightManager_->SetSpotLight(flashlight_->GetSpotLight(), 0);
}

ElevatorFallStage::ElevatorFallStage(Player* player)
{
    player_ = player;


    whiteBoardManager_ = std::make_unique<WhiteBoardManager>(&player_->GetTransform().translate, 10, 0);
    portalManager_ = std::make_unique<FallPortalManager>(&player_->GetTransform().translate);

    portalManager_->SetWhiteBoardManager(whiteBoardManager_.get());

    timeCardWatch_ = std::make_unique<TimeCardWatch>();
    timeCardWatch_->SetPlayer(player_);

    //懐中電灯の作成
    flashlight_ = std::make_unique<Flashlight>();
    flashlight_->SetPlayer(player_);

    wallManager2_ = std::make_unique<WallManager2>();
    testField_ = std::make_unique<TestField>();

    key_ = std::make_unique<Key>();
    door_ = std::make_unique<Door>();

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
    wallManager2_->Initialize();
    testField_->Initialize();
    //衝突しない
    testField_->SetIsCollided(false);

    key_->Initialize();
    door_->Initialize();

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

    //上で判定を取っている
    if (portalManager_->GetIsWarp()) {
        testField_->SetIsCollided(true);
    }

    Vector3 translate = player->GetWorldPosition();
    if (translate.y <= -35.0f) {

        //translate = whiteBoardManager_->GetWhiteBoards().at(1)->GetWorldPosition();
        translate.x = 7.0f;
        translate.y = -5.0f;
        translate.z = -14.0f;
        player->ResetVelocityY(0.0f);
        player->SetTranslate(translate);
    }
    whiteBoardManager_->Update();
    timeCardWatch_->Update();
    //懐中電灯の更新
    flashlight_->Update();
    key_->Update();
    door_->Update();

    wallManager2_->Update();
    testField_->Update();

    UpdateLights();
}

void ElevatorFallStage::UpdatePortal()
{
    portalManager_->Update();
}

void ElevatorFallStage::CheckCollision()
{

    portalManager_->CheckCollision();

    if (!stageCollisionManager_) {
        return;
    }

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


    for (auto& [name, wall] : wallManager2_->GetColliders()) {
        stageCollisionManager_->AddCollider(wall.get());
    }

    stageCollisionManager_->AddCollider(testField_.get());

    stageCollisionManager_->AddCollider(door_->GetAutoLockSystem().get());
    if (!door_->GetIsOpen()) {
        stageCollisionManager_->AddCollider(door_.get());
    }

    stageCollisionManager_->AddCollider(flashlight_.get());
    stageCollisionManager_->AddCollider(key_.get());

    stageCollisionManager_->CheckAllCollisions();
}

void ElevatorFallStage::DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle)
{

    timeCardWatch_->Draw();
    whiteBoardManager_->Draw();
    flashlight_->Draw();
    key_->Draw();
    door_->Draw();

    wallManager2_->Draw();

    testField_->Draw();

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
    wallManager2_->SetCamera(camera);
    testField_->SetCamera(camera);
    key_->SetCamera(camera);
    door_->SetCamera(camera);
}

void ElevatorFallStage::SetPlayerCamera(PlayerCamera* playerCamera)
{
    portalManager_->SetPlayerCamera(playerCamera);
    flashlight_->SetPlayerCamera(playerCamera);
    key_->SetPlayerCamera(playerCamera);
    door_->SetPlayerCamera(playerCamera);
}

PortalManager* ElevatorFallStage::GetPortalManager()
{
    return portalManager_.get();
}

void ElevatorFallStage::SetLightManager(Yoshida::LightManager* lightManager)
{
    lightManager_ = lightManager;
}
