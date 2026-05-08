#include "GentleManStage.h"
#include<assert.h>

#include "GameObject/Door/Door.h"

#include "GameObject/Key/Key.h"
#include "GameObject/Player/Player.h"
#include "GameObject/Portal/PortalManager.h"
#include "GameObject/Gentleman/GiantGentleMan.h"

#include "GameObject/Wall/wallManagerRoofFoor.h"

#include "GameObject/WhiteBoard/WhiteBoardManager.h"
#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include"GameObject/DocumentManager/DocumentManager.h"
#include "Engine/Editor/EditorTool/Hierarchy/Hierarchy.h"

void GentleManStage::InitializeLights()
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

    lightManager_->SetActiveLightCount(Yoshida::LightManager::AREA, 4);

    lightManager_->SetAreaLight(wallManagerRoofFoor_->GetAreaLights().at(0), 0);
    lightManager_->SetAreaLight(wallManagerRoofFoor_->GetAreaLights().at(1), 1);

}

void GentleManStage::UpdateLights()
{

    lightManager_->SetAreaLight(wallManagerRoofFoor_->GetAreaLights().at(0), 0);
    lightManager_->SetAreaLight(wallManagerRoofFoor_->GetAreaLights().at(1), 1);

}

GentleManStage::GentleManStage(Player* player)
{
    player_ = player;

    whiteBoardManager_ = std::make_unique<WhiteBoardManager>(&player_->GetTransform().translate);
    portalManager_ = std::make_unique<PortalManager>(&player_->GetTransform().translate, whiteBoardManager_.get());
    wallManagerRoofFoor_ = std::make_unique<WallManagerRoofFoor>();
    documentManager_ = std::make_unique<DocumentManager>();

    timeCardWatch_ = std::make_unique<TimeCardWatch>();
    timeCardWatch_->SetPlayer(player_);

    //巨大紳士
    giantGentleMan_ = std::make_unique<GiantGentleMan>();

}

void GentleManStage::Initialize()
{

    Hierarchy* hierarchy = Hierarchy::GetInstance();
    hierarchy->BeginRegisterFile("GentleManStage_objectEditors.json");

    InitializeLights();
    whiteBoardManager_->Initialize();
    portalManager_->Initialize();

    wallManagerRoofFoor_->Initialize();
    timeCardWatch_->Initialize();
    documentManager_->Initialize();
    giantGentleMan_->Initialize();

    hierarchy->LoadObjectEditorsFromJsonIfExists("GentleManStage_objectEditors.json");
    hierarchy->EndRegisterFile();
}

void GentleManStage::UpdateGameObject(Camera* camera, const Vector3& lightDirection, Player* player) {
    portalManager_->WarpPlayer(player);
    wallManagerRoofFoor_->Update();
    whiteBoardManager_->Update();
    giantGentleMan_->Update();
    documentManager_->Update(camera, lightDirection);

    UpdateLights();
}

void GentleManStage::UpdatePortal()
{
    portalManager_->Update();
}

void GentleManStage::CheckCollision() {
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
    for (auto& wall : wallManagerRoofFoor_->GetWalls()) {
        stageCollisionManager_->AddCollider(wall.get());
    }

    stageCollisionManager_->AddCollider(giantGentleMan_.get());
    stageCollisionManager_->CheckAllCollisions();
}

void GentleManStage::SetPlayer(Player* player)
{
    player_ = player;
}
void GentleManStage::SetCollisionManager(CollisionManager* collisionManager)
{
    stageCollisionManager_ = collisionManager;
}
void GentleManStage::DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle) {

    wallManagerRoofFoor_->Draw();

    //ここで書類パーティクルを描画させる
    documentManager_->Draw();
    timeCardWatch_->Draw();
    whiteBoardManager_->Draw();
    giantGentleMan_->Draw();
    portalManager_->Draw(isShadow, drawPortal, isDrawParticle);
}
void GentleManStage::DrawSprite()
{
    documentManager_->DrawSprite();

}
void GentleManStage::SetSceneCameraForDraw(Camera* camera) {

    whiteBoardManager_->SetCamera(camera);
    portalManager_->SetCamera(camera);
    wallManagerRoofFoor_->SetCamera(camera);
    documentManager_->SetCamera(camera);
    timeCardWatch_->SetCamera(camera);
    giantGentleMan_->SetCamera(camera);
    
}

void GentleManStage::SetPlayerCamera(PlayerCamera* playerCamera) {
    portalManager_->SetPlayerCamera(playerCamera);
    documentManager_->SetPlayerCamera(playerCamera);
    giantGentleMan_->SetPlayerCamera(playerCamera);
}
PortalManager* GentleManStage::GetPortalManager() { return portalManager_.get(); }

void GentleManStage::SetLightManager(Yoshida::LightManager* lightManager)
{
    lightManager_ = lightManager;

}
