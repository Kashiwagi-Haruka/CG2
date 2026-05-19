#include "FallPortalManager.h"
#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "GameObject/Player/Player.h"
#include "GameObject/SEManager/SEManager.h"
#include "GameObject/WhiteBoard/WhiteBoardManager.h"
#include"GameBase.h"
bool FallPortalManager::canMakePortal_ = false;

FallPortalManager::FallPortalManager(Vector3* pos)
{
    playerPos_ = pos;

    firstWarpPosTransform_ = {
        .scale = {1.0f,  1.0f, 1.0f},
          .rotate = {0.0f, 0.0f, 0.0f},
          .translate = {7.0f, 1.5f, -2.5f}
    };

    portalParticle_ = std::make_unique<PortalParticle>();
}
FallPortalManager::~FallPortalManager() {

    for (auto& portal : portals_) {
        portal.reset();
    }
    portals_.clear();
}


void FallPortalManager::Initialize() {

    canMakePortal_ = false;

    for (auto& portal : portals_) {
        portal.reset();
    }

    portals_.clear();
    portalParticle_->Initialize();

}

void FallPortalManager::SpawnFirstPortal()
{

    SpawnPortal(pendingWhiteBoard_);
}

void FallPortalManager::UpdatePortal()
{
    const float deltaTime = GameBase::GetInstance()->GetDeltaTime();
    warpCoolTimer_ += deltaTime;
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

void FallPortalManager::SpawnPortal(WhiteBoard* board)
{
    // ポータルを新たに作る
    std::unique_ptr<Portal> newPortal = std::make_unique<Portal>();
    newPortal->Initialize();
    // カメラをセットする
    newPortal->SetCamera(playerCamera_->GetCamera());
    newPortal->SetPlayerCamera(playerCamera_->GetCamera());

    auto& firstWhiteBoard = whiteBoardManager_->GetWhiteBoards().at(0);
    assert(firstWhiteBoard);

    if (board != firstWhiteBoard.get()) {
        newPortal->SetCanWarpAngleRange(1.0f);
    }
    newPortal->SetParentTransform(&board->GetCollisionTransform());
    newPortal->SetPortalWorldMatrix();

    if (firstWhiteBoard) {
        newPortal->GetWarpPos()->SetParent(&firstWhiteBoard->GetCollisionTransform());
    } else {
        newPortal->GetWarpPos()->SetParent(&firstWarpPosTransform_);
    }

    portals_.push_back(std::move(newPortal));
    SEManager::SoundPlay(SEManager::PORTAL_SPAWN);
}
