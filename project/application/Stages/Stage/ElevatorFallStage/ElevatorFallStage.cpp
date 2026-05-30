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
#include "GameObject/Wall/WallManagerElevatorFall.h"
#include "GameObject/TestField/TestField.h"
#include "GameObject/Door/Door.h"
#include"GameObject/SEManager/SEManager.h"
#include "GameObject/Key/Key.h"
#include "GameObject/Desk/DeskManager.h"
#include "GameObject/Locker/LockerManager.h"

void ElevatorFallStage::InitializeLights()
{
    assert(lightManager_);
    lightManager_->ClearLights();
    lightManager_->Initialize();
    lightManager_->SetActiveLightCount(Yoshida::LightManager::POINT, 3 + 6);

    PointCommonLight pointLight;
    pointLight.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    pointLight.position = { 7.0f, -7.0f,-18.0f };
    pointLight.intensity = 1.0f;
    pointLight.radius = 14.0f;
    pointLight.decay = 1.0f;

    for (int i = 0; i < 6; ++i) {
        pointLight.position = { 7.0f, (i+1)* - 7.0f,-18.0f};
        lightManager_->SetPointLight(pointLight, 3+i);
    }

    lightManager_->SetActiveLightCount(Yoshida::LightManager::AREA, 3);

    lightManager_->SetAreaLight(wallManager2_->GetAreaLights().at(0), 1);
    lightManager_->SetAreaLight(wallManager2_->GetAreaLights().at(1), 2);

    lightManager_->SetActiveLightCount(Yoshida::LightManager::SPOT, 1);
    lightManager_->SetSpotLight(flashlight_->GetSpotLight(), 0);
}

void ElevatorFallStage::UpdateLights()
{
    lightManager_->SetAreaLight(wallManager2_->GetAreaLights().at(0), 1);
    lightManager_->SetAreaLight(wallManager2_->GetAreaLights().at(1), 2);

    lightManager_->SetSpotLight(flashlight_->GetSpotLight(), 0);
}

ElevatorFallStage::ElevatorFallStage(Player* player)
{
    player_ = player;


    whiteBoardManager_ = std::make_unique<WhiteBoardManager>(&player_->GetTransform().translate, 10, 0);
    portalManager_ = std::make_unique<FallPortalManager>(&player_->GetTransform().translate);

    portalManager_->SetWhiteBoardManager(whiteBoardManager_.get());


    //懐中電灯の作成
    flashlight_ = std::make_unique<Flashlight>();
    flashlight_->SetPlayer(player_);

    wallManager2_ = std::make_unique<WallManager2>();
    wallManagerElevatorFall_ = std::make_unique<WallManagerElevatorFall>();
    testField_ = std::make_unique<TestField>();

    key_ = std::make_unique<Key>();
    door_ = std::make_unique<Door>();

    deskManager_ = std::make_unique<DeskManager>(8);
    lockerManager_ = std::make_unique<LockerManager>(8);
}

void ElevatorFallStage::Initialize()
{
    Hierarchy* hierarchy = Hierarchy::GetInstance();
    hierarchy->BeginRegisterFile("ElevatorFallStage_objectEditors.json");

    InitializeLights();
    portalManager_->Initialize();
    whiteBoardManager_->Initialize();


    //懐中電灯の初期化
    flashlight_->Initialize();
    wallManager2_->Initialize();
    wallManagerElevatorFall_->Initialize();
    testField_->Initialize();
    //衝突しない
    testField_->SetIsCollided(false);

    key_->Initialize();
    door_->Initialize();
    deskManager_->Initialize();
    lockerManager_->Initialize();

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
        if (SEManager::IsSoundFinished(SEManager::FOOT_STEP)) {
            //着地音を鳴らす
            SEManager::SoundPlay(SEManager::FOOT_STEP);
        }
     
        testField_->SetIsCollided(true);
    }

    Vector3 translate = player->GetWorldPosition();
    if (translate.y <= -40.0f) {

        //translate = whiteBoardManager_->GetWhiteBoards().at(1)->GetWorldPosition();
        translate.x = 7.0f;
        translate.y = -5.0f;
        translate.z = -16.0f;
        player->ResetVelocityY(0.0f);
        player->SetTranslate(translate);
    }

    whiteBoardManager_->Update();

    //懐中電灯の更新
    flashlight_->Update();
    key_->Update();

    door_->Update();

    wallManager2_->Update();
    wallManagerElevatorFall_->Update();
    testField_->Update();
    deskManager_->Update();

    lockerManager_->Update();
  /*  lockerManager_->InLocker(player);*/

    UpdateLights();
}

void ElevatorFallStage::UpdatePortal()
{
    portalManager_->Update();
}

void ElevatorFallStage::CheckCollision()
{

    portalManager_->CheckCollision(true);

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


    for (auto& [name, wall] : wallManagerElevatorFall_->GetColliders()) {
        stageCollisionManager_->AddCollider(wall.get());
    }

    for (auto& desk : deskManager_->GetDesks()) {
        stageCollisionManager_->AddCollider(desk.get());
    }

    for (auto& locker : lockerManager_->GetLockers()) {
        for (auto& [name, collision] : locker->GetColliders()) {
            //開いていないときFrontのColliderを追加する
            bool flag = locker->GetIsOpen() && name.find("Front") != std::string::npos;

            if (!flag && !locker->GetIsPlayerIn()) {
                stageCollisionManager_->AddCollider(collision.get());
            }

        }
        if (!locker->GetIsPlayerIn()) {
            stageCollisionManager_->AddCollider(locker.get());
        }

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

    whiteBoardManager_->Draw();
    flashlight_->Draw();
    key_->Draw();
    door_->Draw();
    deskManager_->Draw();
    lockerManager_->Draw();

    wallManager2_->Draw();
    wallManagerElevatorFall_->Draw();

    if (testField_->GetIsCollided()) {
        testField_->Draw();
    }


    portalManager_->Draw(isShadow, drawPortal, isDrawParticle);
}

void ElevatorFallStage::DrawSprite()
{
    lockerManager_->DrawSprite();
}

void ElevatorFallStage::SetSceneCameraForDraw(Camera* camera)
{
    portalManager_->SetCamera(camera);
    flashlight_->SetCamera(camera);
    whiteBoardManager_->SetCamera(camera);
    wallManager2_->SetCamera(camera);
    wallManagerElevatorFall_->SetCamera(camera);
    testField_->SetCamera(camera);
    key_->SetCamera(camera);
    door_->SetCamera(camera);
    deskManager_->SetCamera(camera);
    lockerManager_->SetCamera(camera);
}

void ElevatorFallStage::SetPlayerCamera(PlayerCamera* playerCamera)
{
    portalManager_->SetPlayerCamera(playerCamera);
    flashlight_->SetPlayerCamera(playerCamera);
    key_->SetPlayerCamera(playerCamera);
    door_->SetPlayerCamera(playerCamera);
    deskManager_->SetPlayerCamera(playerCamera);
    lockerManager_->SetPlayerCamera(playerCamera);
}

PortalManager* ElevatorFallStage::GetPortalManager()
{
    return portalManager_.get();
}

void ElevatorFallStage::SetLightManager(Yoshida::LightManager* lightManager)
{
    lightManager_ = lightManager;
}
