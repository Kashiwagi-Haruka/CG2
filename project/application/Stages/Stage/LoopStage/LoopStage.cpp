#include "LoopStage.h"
#include "GameObject/Player/Player.h"
#include "GameObject/YoshidaMath/CollisionManager/ObjectCollider.h"
#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include "Engine/Editor/EditorTool/Hierarchy/Hierarchy.h"
#include "GameObject/Flashlight/Flashlight.h"
#include "MiniMap/MiniMap.h"
#include "GameObject/Portal/SpherePortalManager/SpherePortalManager.h"
#include "GameObject/GentleMan/GiantEnemyManager.h"
#include"TextureManager.h"
#include"GameObject/File/FileManager.h"

void LoopStage::InitializeLights()
{
    assert(lightManager_);
    lightManager_->ClearLights();
    lightManager_->Initialize();

    lightManager_->SetActiveLightCount(Yoshida::LightManager::POINT, 3);

    lightManager_->SetActiveLightCount(Yoshida::LightManager::SPOT, 1);
    lightManager_->SetSpotLight(flashlight_->GetSpotLight(), 0);

    lightManager_->SetActiveLightCount(Yoshida::LightManager::AREA, 1);

}

void LoopStage::UpdateLights()
{
    lightManager_->SetSpotLight(flashlight_->GetSpotLight(), 0);
}

LoopStage::LoopStage(Player* player)
{
    player_ = player;

    //懐中電灯の作成
    flashlight_ = std::make_unique<Flashlight>();
    flashlight_->SetPlayer(player_);

    giantEnemyManager_ = std::make_unique<GiantEnemyManager>(&player_->GetTransform().translate, 3);
    portalManager_ = std::make_unique<SpherePortalManager>(&player_->GetTransform().translate);

    fieldCollider_["LoopStageField"] = std::make_unique<ObjectCollider>();
    fieldCollider_["LoopStageWall_L"] = std::make_unique<ObjectCollider>();
    fieldCollider_["LoopStageWall_R"] = std::make_unique<ObjectCollider>();
    fieldCollider_["LoopStageWall_B"] = std::make_unique<ObjectCollider>();
    fieldCollider_["LoopStageWall_F"] = std::make_unique<ObjectCollider>();


    portalManager_->SetGiantEnemyManager(giantEnemyManager_.get());

    fileManager_ = std::make_unique<FileManager>(20);
}

void LoopStage::Initialize()
{

    Hierarchy* hierarchy = Hierarchy::GetInstance();
    hierarchy->BeginRegisterFile("LoopStage_objectEditors.json");


    portalManager_->Initialize();
    giantEnemyManager_->Initialize();


    // 懐中電灯の初期化
    flashlight_->Initialize();

    fileManager_->Initialize();

    //コライダーの設定
    uint32_t textureHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/floor.png");
    uint32_t wallTextureHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/wall.png");

    for (auto& [name, collider] : fieldCollider_) {
        fieldCollider_[name]->Initialize(YoshidaMath::kAABB);
        fieldCollider_[name]->RegisterEditor(name);
        if (name == "LoopStageField") {
            fieldCollider_[name]->SetCollisionAttribute(kCollisionFloor);
            fieldCollider_[name]->SetTextureIndex(textureHandle);
        } else {
            fieldCollider_[name]->SetTextureIndex(wallTextureHandle);
        }
    }



    InitializeLights();

    MiniMap* miniMap = MiniMap::GetInstance();
    //miniMap->AddObject(wallManagerRestRoom_->GetRoom().get(), { 0.2f, 0.2f, 0.2f, 0.35f }, 50.0f);

    hierarchy->LoadObjectEditorsFromJsonIfExists("LoopStage_objectEditors.json");
    hierarchy->EndRegisterFile();
}

void LoopStage::SetPlayer(Player* player)
{
    player_ = player;
    flashlight_->SetPlayer(player_);


}

void LoopStage::SetCollisionManager(CollisionManager* collisionManager)
{
    stageCollisionManager_ = collisionManager;
}

void LoopStage::UpdateGameObject(Camera* camera, const Vector3& lightDirection, Player* player)
{
    //ポータルでワープする
    portalManager_->WarpPlayer(player);
    giantEnemyManager_->Update();
    portalManager_->Update();
    //懐中電灯の更新
    flashlight_->Update();
    fileManager_->Update();

    for (auto& [name, collider] : fieldCollider_) {
        fieldCollider_[name]->Update();
    }

    UpdateLights();
}

void LoopStage::UpdatePortal()
{
    portalManager_->Update();


}

void LoopStage::CheckCollision()
{
    if (fieldCollider_["LoopStageField"]) {
        //ここに足のSE追加
        giantEnemyManager_->CheckFloorCollision(fieldCollider_["LoopStageField"].get());
    }

    // ==================//ポータル================================
    portalManager_->CheckCollision();
    // ==================//ポータル================================

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

    for (auto& enemy : giantEnemyManager_->GetEnemies()) {
        for (auto& [name, collider] : enemy->GetColliders()) {
            stageCollisionManager_->AddCollider(collider.get());
        }
    }

    for (auto& [name, collider] : fieldCollider_) {
        stageCollisionManager_->AddCollider(collider.get());       
    }



    stageCollisionManager_->AddCollider(flashlight_.get());

    stageCollisionManager_->CheckAllCollisions();


}

void LoopStage::DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle)
{

    flashlight_->Draw();

    //for (auto& [name, collider] : fieldCollider_) {
    //    collider->Draw();
    //}

    fieldCollider_["LoopStageField"]->Draw();

    giantEnemyManager_->Draw();

    Object3dCommon::GetInstance()->DrawCommon();
    fileManager_->Draw();

    portalManager_->Draw(isShadow, drawPortal, isDrawParticle);
}

void LoopStage::DrawSprite()
{
}

void LoopStage::SetSceneCameraForDraw(Camera* camera)
{
    portalManager_->SetCamera(camera);
    giantEnemyManager_->SetCamera(camera);

    flashlight_->SetCamera(camera);
    
    fileManager_->SetCamera(camera);

 /*   for (auto& [name, collider] : fieldCollider_) {
        collider->SetCamera(camera);
    }  */ 
    fieldCollider_["LoopStageField"]->SetCamera(camera);

}

void LoopStage::SetPlayerCamera(PlayerCamera* playerCamera)
{
    portalManager_->SetPlayerCamera(playerCamera);
    flashlight_->SetPlayerCamera(playerCamera);

}

PortalManager* LoopStage::GetPortalManager()
{
    return portalManager_.get();
}

void LoopStage::SetLightManager(Yoshida::LightManager* lightManager)
{
    lightManager_ = lightManager;
}
