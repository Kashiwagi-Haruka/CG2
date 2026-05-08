#include "PortalManager.h"

#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "GameObject/Player/Player.h"
#include "GameObject/SEManager/SEManager.h"
#include "GameObject/WhiteBoard/WhiteBoardManager.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"
#include "Function.h"

bool PortalManager::canMakePortal_ = false;

PortalManager::PortalManager(Vector3* pos) {

    playerPos_ = pos;
  
    firstWarpPosTransform_ = {
        .scale = {1.0f,  1.0f, 1.0f},
          .rotate = {0.0f, 0.0f, 0.0f},
          .translate = {-4.0f, 1.5f, 5.5f}
    };

    portalParticle_ = std::make_unique<PortalParticle>();
}

PortalManager::~PortalManager() {

    for (auto& portal : portals_) {
        portal.reset();
    }
    portals_.clear();
}

void PortalManager::SetWhiteBoardManager(WhiteBoardManager* whiteBoardManager)
{
    whiteBoardManager_ = whiteBoardManager;

}

void PortalManager::Initialize() {

    canMakePortal_ = false;
    warpCoolTimer_ = kWarpTime_;
    
    for (auto& portal : portals_) {
        portal.reset();
    }

    portals_.clear();
    portalParticle_->Initialize();


}

void PortalManager::SpawnFirstPortal()
{

    SpawnPortal(pendingWhiteBoard_);
}

void PortalManager::WarpPlayer(Player* player) {

    for (auto& portal : portals_) {
        //プレイヤーがヒットしているとき且つポータルと向き合っているとき
        if (portal->GetIsPlayerCanWarp()) {
            if (warpCoolTimer_ == kWarpTime_) {
                warpCoolTimer_ = 0.0f;

                Transform transform = *portal->GetWarpPos()->GetParent();
                transform.translate.y = 0.0f;
                player->SetTranslate(transform.translate);
                player->SetRotate(portal->GetWarpPos()->GetTransform().rotate + transform.rotate);
                SEManager::SoundPlay(SEManager::WARP);
                break;
            }
        }
    }
}

void PortalManager::UpdatePortal() {

    warpCoolTimer_ += YoshidaMath::kDeltaTime;
    warpCoolTimer_ = std::clamp(warpCoolTimer_, 0.0f, kWarpTime_);

    if (isPendingPortalSpawn_ && portalParticle_) {
        portalParticle_->Update();
        if (portalParticle_->IsFinished() && pendingWhiteBoard_) {
            SpawnPortal(pendingWhiteBoard_);
            pendingWhiteBoard_ = nullptr;
            isPendingPortalSpawn_ = false;
        }
    }

    for (auto& portal : portals_) {
        portal->Update();
    }
}

void PortalManager::SetCamera(Camera* camera) {

    if (portalParticle_) {
        portalParticle_->SetCamera(camera);
    }
};

void PortalManager::DrawPortal() {

    for (auto& portal : portals_) {
        portal->DrawRings();

        portal->DrawPortals();
    }
}

void PortalManager::Draw(bool isShadow, bool drawPortal, bool drawParticle) {

    if (drawPortal) {
        DrawPortal();
    }

    for (auto& portal : portals_) {
        portal->GetWarpPos()->Draw();
    }

    if (drawParticle && portalParticle_) {
        portalParticle_->Draw();
    }
}

void PortalManager::SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; }

void PortalManager::Update() {

    UpdatePortal();
}

void PortalManager::CheckCollision() {

    if (isPendingPortalSpawn_) {
        return;
    }

    if (!whiteBoardManager_ || !playerCamera_) {
        canMakePortal_ = false;
        return;
    }

    WhiteBoard* hitBoard = whiteBoardManager_->CheckCollision(playerCamera_);
    canMakePortal_ = whiteBoardManager_->GetCanMakePortal();

    if (!hitBoard) {
        return;
    }

    if (portals_.size() >= 2) {
        // ポータルの生成が2個以上になったら
        portals_.erase(portals_.begin());
    }

    pendingWhiteBoard_ = hitBoard;
    isPendingPortalSpawn_ = true;

    if (portalParticle_) {
        portalParticle_->Start(*playerPos_, pendingWhiteBoard_->GetCollisionTransform().translate);
    }
}

void PortalManager::SpawnPortal(WhiteBoard* board) {

    // ポータルを新たに作る
    std::unique_ptr<Portal> newPortal = std::make_unique<Portal>();
    newPortal->Initialize();
    // カメラをセットする
    newPortal->SetCamera(playerCamera_->GetCamera());
    newPortal->SetPlayerCamera(playerCamera_->GetCamera());
    newPortal->SetParentTransform(&board->GetCollisionTransform());
    newPortal->SetPortalWorldMatrix();

    if (!portals_.empty()) {
        // すでにポータルがある場合、お互いをつなぐ
        Portal* existingPortal = portals_.back().get();
        newPortal->GetWarpPos()->SetParent(&existingPortal->GetTransform());
        existingPortal->GetWarpPos()->SetParent(&newPortal->GetTransform());
    } else {
        // ポータルがないとき
        newPortal->GetWarpPos()->SetParent(&firstWarpPosTransform_);
    }
    portals_.push_back(std::move(newPortal));
    SEManager::SoundPlay(SEManager::PORTAL_SPAWN);
}