#include "RestroomStage.h"
#include<assert.h>

#include "GameObject/Key/Key.h"
#include "GameObject/Door/Door.h"
#include "GameObject/Player/Player.h"
#include "GameObject/Portal/GentlemanPortalManager.h"

#include"GameObject/Wall/WallManagerRestRoom.h"

#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"

#include "Engine/Editor/EditorTool/Hierarchy/Hierarchy.h"
#include"Function.h"
#include "GameObject/Flashlight/Flashlight.h"
#include "GameObject/WhiteBoard/WhiteBoardManager.h"
#include"GameObject/Toilet/ToiletManager.h"

void RestroomStage::InitializeLights()
{
    assert(lightManager_);
    lightManager_->ClearLights();
    lightManager_->Initialize();
    lightManager_->SetActiveLightCount(Yoshida::LightManager::POINT, 2);

    PointCommonLight pointLights_[2];
    pointLights_[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    pointLights_[0].position = { 7.0f, 5.0f, 0.0f };
    pointLights_[0].intensity = 1.0f;
    pointLights_[0].radius = 10.0f;
    pointLights_[0].decay = 1.0f;

    pointLights_[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    pointLights_[1].position = { -7.0f, 5.0f, 0.0f };
    pointLights_[1].intensity = 1.0f;
    pointLights_[1].radius = 10.0f;
    pointLights_[1].decay = 1.0f;
    lightManager_->SetPointLight(pointLights_[0], 0);
    lightManager_->SetPointLight(pointLights_[1], 1);



    lightManager_->SetActiveLightCount(Yoshida::LightManager::SPOT, 1);
    lightManager_->SetSpotLight(flashlight_->GetSpotLight(), 0);

    lightManager_->SetActiveLightCount(Yoshida::LightManager::AREA, 4);

    lightManager_->SetAreaLight(wallManagerRestRoom_->GetAreaLights().at(0), 0);
    lightManager_->SetAreaLight(wallManagerRestRoom_->GetAreaLights().at(1), 1);

}

void RestroomStage::UpdateLights()
{

    lightManager_->SetSpotLight(flashlight_->GetSpotLight(), 0);

    lightManager_->SetAreaLight(wallManagerRestRoom_->GetAreaLights().at(0), 0);
    lightManager_->SetAreaLight(wallManagerRestRoom_->GetAreaLights().at(1), 1);

}

RestroomStage::RestroomStage(Player* player)
{
    player_ = player;


    whiteBoardManager_ = std::make_unique<WhiteBoardManager>(&player_->GetTransform().translate);
    portalManager_ = std::make_unique<GentlemanPortalManager>(&player_->GetTransform().translate);

    portalManager_->SetWhiteBoardManager(whiteBoardManager_.get());
    wallManagerRestRoom_ = std::make_unique<WallManagerRestRoom>();

    key_ = std::make_unique<Key>();
    door_ = std::make_unique<Door>();

    timeCardWatch_ = std::make_unique<TimeCardWatch>();
    timeCardWatch_->SetPlayer(player_);
    //懐中電灯の作成
    flashlight_ = std::make_unique<Flashlight>();
    flashlight_->SetPlayer(player_);

    toiletManager_ = std::make_unique<ToiletManager>();
}

void RestroomStage::Initialize()
{

    Hierarchy* hierarchy = Hierarchy::GetInstance();
    hierarchy->BeginRegisterFile("RestroomStage_objectEditors.json");

    InitializeLights();
    portalManager_->Initialize();
    whiteBoardManager_->Initialize();
    wallManagerRestRoom_->Initialize();
    timeCardWatch_->Initialize();
    //懐中電灯の初期化
    flashlight_->Initialize();
    key_->Initialize();
    door_->Initialize();
    toiletManager_->Initialize();

    hierarchy->LoadObjectEditorsFromJsonIfExists("RestroomStage_objectEditors.json");
    hierarchy->EndRegisterFile();
}

void RestroomStage::UpdateGameObject(Camera* camera, const Vector3& lightDirection, Player* player) {

    portalManager_->WarpPlayer(player);
    wallManagerRestRoom_->Update();

    whiteBoardManager_->Update();

    key_->Update();
    door_->Update();
    toiletManager_->Update();
    //懐中電灯の更新
    flashlight_->Update();
    UpdateLights();
}

void RestroomStage::UpdatePortal()
{
    portalManager_->Update();
}

void RestroomStage::CheckCollision() {
    if (!stageCollisionManager_) {
        return;
    }

    portalManager_->CheckCollision();
    door_->CheckCollision();

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

    //for (auto& wall : wallManagerRestRoom_->GetWalls()) {
    //    stageCollisionManager_->AddCollider(wall.get());
    //}
    for (auto& [name, wall] : wallManagerRestRoom_->GetColliders()) {
        stageCollisionManager_->AddCollider(wall.get());
    }

    //便器
    for (auto& toilet : toiletManager_->GetToilets()) {
        stageCollisionManager_->AddCollider(toilet.get());
    }

    stageCollisionManager_->AddCollider(door_->GetAutoLockSystem().get());
    if (!door_->GetIsOpen()) {
        stageCollisionManager_->AddCollider(door_.get());
    }

    stageCollisionManager_->AddCollider(flashlight_.get());

    stageCollisionManager_->CheckAllCollisions();
}

void RestroomStage::SetPlayer(Player* player)
{
    player_ = player;
    flashlight_->SetPlayer(player_);
}
void RestroomStage::SetCollisionManager(CollisionManager* collisionManager)
{
    stageCollisionManager_ = collisionManager;
}
void RestroomStage::DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle) {

    wallManagerRestRoom_->Draw();
    key_->Draw();
    timeCardWatch_->Draw();
    door_->Draw();
    whiteBoardManager_->Draw();
    flashlight_->Draw();

    toiletManager_->Draw();


    portalManager_->Draw(isShadow, drawPortal, isDrawParticle);
}
void RestroomStage::DrawSprite()
{


}
void RestroomStage::SetSceneCameraForDraw(Camera* camera) {

    portalManager_->SetCamera(camera);
    wallManagerRestRoom_->SetCamera(camera);
    timeCardWatch_->SetCamera(camera);
    flashlight_->SetCamera(camera);
    whiteBoardManager_->SetCamera(camera);
    key_->SetCamera(camera);
    door_->SetCamera(camera);
    toiletManager_->SetCamera(camera);
}

void RestroomStage::SetPlayerCamera(PlayerCamera* playerCamera) {
    portalManager_->SetPlayerCamera(playerCamera);
    flashlight_->SetPlayerCamera(playerCamera);
    key_->SetPlayerCamera(playerCamera);
    door_->SetPlayerCamera(playerCamera);
    toiletManager_->SetPlayerCamera(playerCamera);
}
PortalManager* RestroomStage::GetPortalManager() { return portalManager_.get(); }

void RestroomStage::SetLightManager(Yoshida::LightManager* lightManager)
{
    lightManager_ = lightManager;

}
