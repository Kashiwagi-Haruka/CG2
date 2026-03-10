#include "PortalManager.h"
#include "Object3d/Object3dCommon.h"
#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "GameObject/KeyBindConfig.h"
#include "GameObject/TimeCard/TimeCardWatch.h"
#include "GameObject/WhiteBoard/WalkWhiteBoard.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"
#include "Model/ModelManager.h"
#include"TextureManager.h"
#include"DirectXCommon.h"
#include"GameObject/Player/Player.h"

namespace {
    const constexpr uint32_t kMaxWhiteBoards = 6;
}

PortalManager::PortalManager(Vector3* pos) {

    playerPos_ = pos;

    firstWarpPosTransform_ = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f,0.0f,0.0f},.translate = { 10.0f, 1.5f, 5.0f } };

    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/whiteBoard", "whiteBoard");
    std::unique_ptr<WalkWhiteBoard> walkWhite = std::make_unique<WalkWhiteBoard>();
    WalkWhiteBoard::LoadAnimation("Resources/TD3_3102/3d/whiteBoard", "whiteBoard");
    walkWhite->SetModel("whiteBoard");
    walkWhite->SetTargetPosPtr(pos);
    whiteBoards_.push_back(std::move(walkWhite));

    for (int i = 0; i < kMaxWhiteBoards; ++i) {
        std::unique_ptr<WhiteBoard> white = std::make_unique<WhiteBoard>();
        white->SetModel("whiteBoard");
        whiteBoards_.push_back(std::move(white));
    }

    portalParticle_ = std::make_unique<PortalParticle>();
}

void PortalManager::Initialize() {

    for (auto& board : whiteBoards_) {
        board->Initialize();
    }
    warpCoolTimer_ = kWarpTime_;
    portals_.clear();
    preWhiteBoards_.clear();
    portalParticle_->Initialize();
}



void PortalManager::WarpPlayer(Player* player)
{

    for (auto& portal : portals_) {
        if (portal->GetIsPlayerHit()) {
            if (warpCoolTimer_ == kWarpTime_) {
                warpCoolTimer_ = 0.0f;
                Transform* portalTransform = portal->GetWarpPos()->GetParent();
                player->SetTranslate(portalTransform->translate);
                player->SetRotate(portalTransform->rotate);
                break;
            }
        }
    }
}

void PortalManager::UpdateWhiteBoard() {
    for (auto& board : whiteBoards_) {
        board->Update();
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

void PortalManager::SetCamera(Camera* camera)
{
    for (auto& board : whiteBoards_) {
        board->SetCamera(camera);
    }

    if (portalParticle_) {
        portalParticle_->SetCamera(camera);
    }

    //for (auto& portal : portals_) {
    //    portal->SetCamera(camera);
    //}                                    
};

void PortalManager::DrawWhiteBoard() {
    for (auto& board : whiteBoards_) {
        board->Draw();
    }
}

void PortalManager::DrawPortal()
{
    for (auto& portal : portals_) {
        portal->DrawRings();
        Object3dCommon::GetInstance()->DrawCommonPortal();
        portal->DrawPortals();
    }


}


void PortalManager::Draw(bool isShadow, bool drawPortal, bool drawParticle) {

    DrawWhiteBoard();

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

void PortalManager::SetPlayerCamera(PlayerCamera* camera) {

    playerCamera_ = camera;
}

void PortalManager::Update()
{
    UpdateWhiteBoard();
    UpdatePortal();
}

void PortalManager::CheckCollision(TimeCardWatch* timeCardWatch) {

    if (isPendingPortalSpawn_) {
        return;
    }

    // whiteBoardとrayの当たり判定
    for (auto& board : whiteBoards_) {
        if (timeCardWatch->OnCollisionObjOfMakePortal(playerCamera_->GetRay(), board->GetAABB(), board->GetCollisionTransform())) {

            if (PlayerCommand::GetInstance()->Shot()) {

                if (preWhiteBoards_.size() >= 2) {
                    //ポータルの生成が2個以上になったら
                    preWhiteBoards_.at(0)->ResetCollisionAttribute();
                    preWhiteBoards_.erase(preWhiteBoards_.begin());
                }

                if (portals_.size() >= 2) {
                    //ポータルの生成が2個以上になったら
                    portals_.erase(portals_.begin());
                }

                preWhiteBoards_.push_back(board.get());

                preWhiteBoards_.back()->SetCollisionAttribute(kCollisionNone);

                pendingWhiteBoard_ = preWhiteBoards_.back();

                isPendingPortalSpawn_ = true;

                if (portalParticle_) {
                    portalParticle_->Start(*playerPos_, preWhiteBoards_.back()->GetCollisionTransform().translate);
                }
            }
            break;
        };
    }
}

void PortalManager::SpawnPortal(WhiteBoard* board) {

    //ポータルを新たに作る
    std::unique_ptr<Portal> newPortal = std::make_unique<Portal>();
    newPortal->Initialize();
    //カメラをセットする
    newPortal->SetCamera(playerCamera_->GetCamera());
    newPortal->SetParentTransform(&board->GetCollisionTransform());
    newPortal->SetPortalWorldMatrix();

    if (!portals_.empty()) {
        // すでにポータルがある場合、お互いをつなぐ
        Portal* existingPortal = portals_.back().get();
        newPortal->GetWarpPos()->SetParent(&existingPortal->GetTransform());
        existingPortal->GetWarpPos()->SetParent(&newPortal->GetTransform());
        //テクスチャの入れ替え
        //existingPortal->SetTextureIndex(existingPortal->GetRenderTexture2D()->GetSrvIndex());
        //newPortal->SetTextureIndex(newPortal->GetRenderTexture2D()->GetSrvIndex());

    } else {
        //ポータルがないとき
        newPortal->GetWarpPos()->SetParent(&firstWarpPosTransform_);
    }


    portals_.push_back(std::move(newPortal));

}