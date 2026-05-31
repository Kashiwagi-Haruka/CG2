#include "SpherePortalManager.h"
#include"GameBase.h"
#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "GameObject/SEManager/SEManager.h"
#include"GameObject/GentleMan/GiantEnemyManager.h"
#include"GameObject/KeyBindConfig.h"

SpherePortalManager::SpherePortalManager(Vector3* pos)
{
    playerPos_ = pos;
    portalParticle_ = std::make_unique<PortalParticle>();

    firstWarpPosTransform_ = {
    .scale = {1.0f,  1.0f, 1.0f},
      .rotate = {0.0f, Function::kPi, 0.0f},
      .translate = {0.0f, 1.5f, 0.0f}
    };
}

SpherePortalManager::~SpherePortalManager()
{
    for (auto& portal : portals_) {
        portal.reset();
    }
    portals_.clear();
}

void SpherePortalManager::Initialize()
{
    canMakePortal_ = false;
    pendingGiantEnemy_ = nullptr;

    for (auto& portal : portals_) {
        portal.reset();
    }

    portals_.clear();
    portalParticle_->Initialize();

    ////最初のエネミーを取得する
    //pendingGiantEnemy_ = giantEnemyManager_->GetEnemies().at(0).get();

    //if (pendingGiantEnemy_) {
    //    SpawnPortal(pendingGiantEnemy_);
    //    isPendingPortalSpawn_ = true;
    //}

}

void SpherePortalManager::UpdatePortal()
{
    const float deltaTime = GameBase::GetInstance()->GetDeltaTime();
    warpCoolTimer_ += deltaTime;
    warpCoolTimer_ = std::clamp(warpCoolTimer_, 0.0f, kWarpTime_);

    if (isPendingPortalSpawn_ && portalParticle_) {
        portalParticle_->Update();
        if (portalParticle_->IsFinished() && pendingGiantEnemy_) {
            SpawnPortal(pendingGiantEnemy_);
            pendingGiantEnemy_ = nullptr;
            isPendingPortalSpawn_ = false;
        }
    }

    for (auto& portal : portals_) {
        portal->Update(true);
    }
}

void SpherePortalManager::CheckCollision(const bool isOneSide)
{

    canMakePortal_ = false;
    canMakePortalToObj_ = false;

    if (isPendingPortalSpawn_) {
        return;
    }

    if (!giantEnemyManager_ || !playerCamera_) {
        return;
    }

    (void)isOneSide;

    GiantEnemy* hitEnemy = giantEnemyManager_->CheckCollision(playerCamera_);

    if (!hitEnemy) {
        return;
    }
    //ここまで来たらホワイトボードにはポータルが作れることが分かるね。
    canMakePortalToObj_ = true;

    //ここまでくるとポータルが作れるよ！
    canMakePortal_ = true;

    //実際に打ってみよう！
    if (!PlayerCommand::GetInstance()->Shot()) {
        return;
    }

    // ショットSE鳴らす
    SEManager::SoundPlay(SEManager::SHOT);

    //ポータルをつくるよん
    giantEnemyManager_->SetPortal(hitEnemy);

    pendingGiantEnemy_ = hitEnemy;
    isPendingPortalSpawn_ = true;

    if (portalParticle_) {
        portalParticle_->Start(playerCamera_->GetTransform().translate, pendingGiantEnemy_->GetCollisionTransform().translate);
    }

}

void SpherePortalManager::SetGiantEnemyManager(GiantEnemyManager* giantEnemyManager)
{
    giantEnemyManager_ = giantEnemyManager;
}

void SpherePortalManager::SpawnPortal(GiantEnemy* gianeEnemy)
{    // ポータルを新たに作る
    std::unique_ptr<Portal> newPortal = std::make_unique<Portal>();
    newPortal->Initialize(PortalMesh::kSphere);
    //ここだけ半径
    newPortal->SetRadius(1.5f);
    // カメラをセットする
    newPortal->SetCamera(playerCamera_->GetCamera());
    newPortal->SetPlayerCamera(playerCamera_->GetCamera());
    newPortal->SetParentTransform(&gianeEnemy->GetCollisionTransform());
    newPortal->SetPortalWorldMatrix();

    if (!portals_.empty()) {
        // すでにポータルがある場合、お互いをつなぐ
        Portal* existingPortal = portals_.back().get();
        newPortal->GetWarpPos()->SetParent(&existingPortal->GetTransform());
        existingPortal->GetWarpPos()->SetParent(&newPortal->GetTransform());
    } else {
        // ポータルがないとき

        newPortal->GetWarpPos()->SetParent(&firstWarpPosTransform_);
        /*newPortal->GetWarpPos()->SetParent(&giantEnemyManager_->GetEnemies().at(0)->GetCollisionTransform());*/
    }

    portals_.push_back(std::move(newPortal));
    SEManager::SoundPlay(SEManager::PORTAL_SPAWN);
}
