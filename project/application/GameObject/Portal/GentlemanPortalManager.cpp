#include "GentlemanPortalManager.h"

#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "GameObject/Player/Player.h"
#include "GameObject/SEManager/SEManager.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"
#include "Function.h"
#include"GameObject/Gentleman/GiantGentleman.h"
#include"GameBase.h"
#include "GameObject/KeyBindConfig.h"

bool GentlemanPortalManager::isMakePortal_ = false;

GentlemanPortalManager::GentlemanPortalManager(Vector3* pos) {

    playerPos_ = pos;

    firstWarpPosTransform_ = {
        .scale = {1.0f,  1.0f, 1.0f},
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
    isMakePortal_ = false;
    warpCoolTimer_ = kWarpTime_;
    isPendingPortalSpawn_ = false;
    isWarp_ = false;

    for (auto& portal : portals_) {
        portal.reset();
    }

    portals_.clear();
    portalParticle_->Initialize();
}

void GentlemanPortalManager::WarpPlayer(Player* player) {

    isWarp_ = false;

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
                isWarp_ = true;
                break;
            }
        }
    }
}

void GentlemanPortalManager::UpdatePortal() {

    const float deltaTime = GameBase::GetInstance()->GetDeltaTime();
    warpCoolTimer_ += deltaTime;
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
    newPortal->Initialize(PortalMesh::kSphere);
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
    }
    portals_.push_back(std::move(newPortal));
    SEManager::SoundPlay(SEManager::PORTAL_SPAWN);
}

void GentlemanPortalManager::DrawPortal() {

    for (auto& portal : portals_) {
        portal->DrawPortals();
    }

    Object3dCommon::GetInstance()->DrawCommon();
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

    Object3dCommon::GetInstance()->DrawCommon();
}

void GentlemanPortalManager::SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; }

void GentlemanPortalManager::Update() {

    UpdatePortal();
}

void GentlemanPortalManager::CheckCollision(const bool isOneSide) {

    (void)isOneSide;

    canMakePortal_ = false;

    if (isMakePortal_) {
        return;
    }

    if (isPendingPortalSpawn_) {
        return;
    }

    if (!giantGentleman_ || !playerCamera_) {

        return;
    }

    canMakePortal_ = giantGentleman_->CanMakePortal();

    if (!canMakePortal_) {
        return;
    }

    if (!PlayerCommand::GetInstance()->Shot()) {
        return;
    }

    // ショットSE鳴らす
    SEManager::SoundPlay(SEManager::SHOT);
    isMakePortal_ = true;

    isPendingPortalSpawn_ = true;

    if (portalParticle_) {
        portalParticle_->Start(*playerPos_, giantGentleman_->GetCollisionTransform().translate);
    }

}
