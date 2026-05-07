#include "TutorialStage.h"
#include<assert.h>

#include "GameObject/Door/Door.h"

#include "GameObject/Key/Key.h"
#include "GameObject/Player/Player.h"
#include "GameObject/Portal/PortalManager.h"
#include "GameObject/TestField/TestField.h"
#include "GameObject/TimeCard/TimeCard.h"
#include "GameObject/TimeCard/TimeCardRack.h"
#include "GameObject/Wall/WallManager.h"
#include "GameObject/Wall/WallManager2.h"
#include "GameObject/WhiteBoard/WhiteBoardManager.h"
#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include"GameObject/DocumentManager/DocumentManager.h"
#include"GameObject/File/FileManager.h"
#include "Engine/Editor/EditorTool/Hierarchy/Hierarchy.h"

void TutorialStage::InitializeLights()
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

    lightManager_->SetAreaLight(wallManager_->GetAreaLights().at(0), 0);
    lightManager_->SetAreaLight(wallManager_->GetAreaLights().at(1), 1);
    lightManager_->SetAreaLight(wallManager2_->GetAreaLights().at(0), 2);
    lightManager_->SetAreaLight(wallManager2_->GetAreaLights().at(1), 3);

}

void TutorialStage::UpdateLights()
{

    lightManager_->SetAreaLight(wallManager_->GetAreaLights().at(0), 0);
    lightManager_->SetAreaLight(wallManager_->GetAreaLights().at(1), 1);
    lightManager_->SetAreaLight(wallManager2_->GetAreaLights().at(0), 2);
    lightManager_->SetAreaLight(wallManager2_->GetAreaLights().at(1), 3);


}

TutorialStage::TutorialStage(Player* player)
{
    player_ = player;

    testField_ = std::make_unique<TestField>();
    whiteBoardManager_ = std::make_unique<WhiteBoardManager>(&player_->GetTransform().translate);
    portalManager_ = std::make_unique<PortalManager>(&player_->GetTransform().translate, whiteBoardManager_.get());

    key_ = std::make_unique<Key>();
    door_ = std::make_unique<Door>();
    wallManager_ = std::make_unique<WallManager>();
    wallManager2_ = std::make_unique<WallManager2>();
    timeCard_ = std::make_unique<TimeCard>();
    timeCardRack_ = std::make_unique<TimeCardRack>();
    documentManager_ = std::make_unique<DocumentManager>();

    fileManager_ = std::make_unique<FileManager>();
}

void TutorialStage::Initialize()
{

    Hierarchy* hierarchy = Hierarchy::GetInstance();
    hierarchy->BeginRegisterFile("TutorialStage_objectEditors.json");


    InitializeLights();

    testField_->Initialize();
    whiteBoardManager_->Initialize();
    portalManager_->Initialize();
    key_->Initialize();
    wallManager_->Initialize();
    wallManager2_->Initialize();
    door_->Initialize();
    timeCard_->Initialize();
    timeCardRack_->Initialize();

    documentManager_->Initialize();
    fileManager_->Initialize();

    hierarchy->LoadObjectEditorsFromJsonIfExists("TutorialStage_objectEditors.json");
    hierarchy->EndRegisterFile();
}

void TutorialStage::UpdateGameObject(Camera* camera, const Vector3& lightDirection, Player* player) {
    portalManager_->WarpPlayer(player);

    key_->Update();
    testField_->Update();
    wallManager_->Update();
    wallManager2_->Update();

    door_->Update();
    whiteBoardManager_->Update();
    timeCard_->Update();
    timeCardRack_->Update();
    documentManager_->Update(camera, lightDirection);
    fileManager_->Update();

    UpdateLights();

}

void TutorialStage::UpdatePortal()
{
    portalManager_->Update();
}

void TutorialStage::CheckCollision() {
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
    for (auto& wall : wallManager_->GetWalls()) {
        stageCollisionManager_->AddCollider(wall.get());
    }
    for (auto& wall : wallManager2_->GetWalls()) {
        stageCollisionManager_->AddCollider(wall.get());
    }

    stageCollisionManager_->AddCollider(testField_.get());

    stageCollisionManager_->AddCollider(door_->GetAutoLockSystem().get());
    if (!door_->GetIsOpen()) {
        stageCollisionManager_->AddCollider(door_.get());
    }
    stageCollisionManager_->AddCollider(key_.get());
    stageCollisionManager_->CheckAllCollisions();
}

void TutorialStage::SetPlayer(Player* player)
{
    player_ = player;
}
void TutorialStage::DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle) {
    testField_->Draw();
    wallManager_->Draw();
    wallManager2_->Draw();
    //ここで書類パーティクルを描画させる
    documentManager_->Draw();

    key_->Draw();
    timeCard_->Draw();
    timeCardRack_->Draw();
    door_->Draw();
    fileManager_->Draw();
    whiteBoardManager_->Draw();

    portalManager_->Draw(isShadow, drawPortal, isDrawParticle);
    

}
void TutorialStage::DrawSprite()
{
    documentManager_->DrawSprite();

}
void TutorialStage::SetSceneCameraForDraw(Camera* camera) {
    testField_->SetCamera(camera);
    whiteBoardManager_->SetCamera(camera);
    portalManager_->SetCamera(camera);
    key_->SetCamera(camera);
    wallManager_->SetCamera(camera);
    wallManager2_->SetCamera(camera);
    door_->SetCamera(camera);
    timeCard_->SetCamera(camera);
    timeCardRack_->SetCamera(camera);
    documentManager_->SetCamera(camera);
    fileManager_->SetCamera(camera);
}

void TutorialStage::SetPlayerCamera(PlayerCamera* playerCamera) {
    portalManager_->SetPlayerCamera(playerCamera);
    key_->SetPlayerCamera(playerCamera);
    door_->SetPlayerCamera(playerCamera);
    documentManager_->SetPlayerCamera(playerCamera);
}
PortalManager* TutorialStage::GetPortalManager() { return portalManager_.get(); }

void TutorialStage::SetLightManager(Yoshida::LightManager* lightManager)
{
    lightManager_ = lightManager;

}void TutorialStage::SetCollisionManager(CollisionManager* collisionManager) {
    stageCollisionManager_ = collisionManager;
}