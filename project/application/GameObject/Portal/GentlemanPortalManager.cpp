#include "GentlemanPortalManager.h"

#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "GameObject/Player/Player.h"
#include "GameObject/SEManager/SEManager.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"
#include "Function.h"
#include"GameObject/Gentleman/GiantGentleman.h"

bool GentlemanPortalManager::canMakePortal_ = false;

GentlemanPortalManager::GentlemanPortalManager(Vector3* pos) {

    playerPos_ = pos;

    firstWarpPosTransform_ = {
        .scale = {2.0f / 1.6f,  2.0f / 0.9f, 1.0f},
          .rotate = {0.0f, 0.0f, 0.0f},
          .translate = {7.0f, -100.0f, 20.0f}
    };

    portalParticle_ = std::make_unique<PortalParticle>();
}

GentlemanPortalManager::~GentlemanPortalManager() {

    for (auto& portal : portals_) {
        portal.reset();
    }

    portals_.clear();
}

void GentlemanPortalManager::SetGentleMan(GiantGentleMan* giantGentleMan)
{
    giantGentleman_ = giantGentleMan;
}

void GentlemanPortalManager::Initialize() {

    canMakePortal_ = false;
    warpCoolTimer_ = kWarpTime_;
    isPendingPortalSpawn_ = false;
    for (auto& portal : portals_) {
        portal.reset();
    }

    portals_.clear();
    portalParticle_->Initialize();
}

void GentlemanPortalManager::WarpPlayer(Player* player) {

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

void GentlemanPortalManager::UpdatePortal() {

    warpCoolTimer_ += YoshidaMath::kDeltaTime;
    warpCoolTimer_ = std::clamp(warpCoolTimer_, 0.0f, kWarpTime_);

    if (isPendingPortalSpawn_ && portalParticle_) {
        portalParticle_->Update();
        if (portalParticle_->IsFinished() && giantGentleman_) {
            SpawnPortal();
            isPendingPortalSpawn_ = false;
        }
    }

    for (auto& portal : portals_) {
        portal->Update();
    }
}

void GentlemanPortalManager::SetCamera(Camera* camera) {

    if (portalParticle_) {
        portalParticle_->SetCamera(camera);
    }
};

void GentlemanPortalManager::SpawnPortal()
{
    // ポータルを新たに作る
    std::unique_ptr<Portal> newPortal = std::make_unique<Portal>();
    newPortal->Initialize();
    //ここだけ半径を多く見積もる
    //newPortal->SetRadius(1.5f);
    newPortal->SetAABB({ .min = {-1.0f,-2.0f,-1.0f},.max = {1.0f,2.0f,1.0f} });
    // カメラをセットする
    newPortal->SetCamera(playerCamera_->GetCamera());
    newPortal->SetPlayerCamera(playerCamera_->GetCamera());
    newPortal->SetParentTransform(&giantGentleman_->GetCollisionTransform());
    newPortal->SetPortalWorldMatrix();

    if (portals_.empty()) {

        // ポータルがないとき
        newPortal->GetWarpPos()->SetParent(&firstWarpPosTransform_);
    } else {
        //// すでにポータルがある場合、お互いをつなぐ
          //Portal* existingPortal = portals_.back().get();
          //newPortal->GetWarpPos()->SetParent(&existingPortal->GetTransform());
          //existingPortal->GetWarpPos()->SetParent(&newPortal->GetTransform());

    }
    portals_.push_back(std::move(newPortal));
    SEManager::SoundPlay(SEManager::PORTAL_SPAWN);
}

void GentlemanPortalManager::DrawPortal() {

    for (auto& portal : portals_) {
        portal->DrawRings();
        portal->DrawPortals();
    }
}

void GentlemanPortalManager::Draw(bool isShadow, bool drawPortal, bool drawParticle) {

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

void GentlemanPortalManager::SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; }

void GentlemanPortalManager::Update() {

    UpdatePortal();
}

void GentlemanPortalManager::CheckCollision() {

    if (isPendingPortalSpawn_) {
        return;
    }

    if (!giantGentleman_ || !playerCamera_) {
        canMakePortal_ = false;
        return;
    }

    canMakePortal_ = giantGentleman_->CanMakePortal();

    if (!canMakePortal_) {
        return;
    }

    if (giantGentleman_->IsMakePortal()) {

        //if (portals_.size() >= 2) {
        //    // ポータルの生成が2個以上になったら
        //    portals_.erase(portals_.begin());
        //}

        isPendingPortalSpawn_ = true;

        if (portalParticle_) {
            portalParticle_->Start(*playerPos_, giantGentleman_->GetCollisionTransform().translate);
        }
    }

}
