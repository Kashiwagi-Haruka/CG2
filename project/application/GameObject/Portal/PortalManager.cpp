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

    portals_.clear();
    preWhiteBoards_.clear();
    portalParticle_->Initialize();
}

void PortalManager::UpdateWarpPosCameras()
{

    for (auto& portal : portals_) {
        portal->UpdateWarpPosCamera();
    }
}

void PortalManager::UpdateWhiteBoard() {
    for (auto& board : whiteBoards_) {
        board->Update();
    }
}

void PortalManager::UpdatePortal() {

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


    if (portals_.size() >= 2) {
           //ポータルの生成が2個以上になったら
        portals_.erase(portals_.begin());
    }

    //ポータルを新たに作る
    std::unique_ptr newPortal = std::make_unique<Portal>();
    newPortal->Initialize();
    //カメラをセットする
    newPortal->SetCamera(playerCamera_->GetCamera());
    newPortal->SetParentTransform(&board->GetCollisionTransform());

    if (portals_.empty()) {
        //ポータルがないとき
        newPortal->GetWarpPos()->SetParent(&firstWarpPosTransform_);
        uint32_t textureIndex = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/atHome.jpg");
        newPortal->SetTextureIndex(textureIndex);
    } else {
        //ポータルがあるとき
        auto& firstPortal = portals_.at(0);

        auto* newPortalTransform = &newPortal->GetTransform();
        uint32_t newPortalSRV = newPortal->GetRenderTexture2D()->GetSrvIndex();
        uint32_t firstPortalSRV = firstPortal->GetRenderTexture2D()->GetSrvIndex();

        firstPortal->SetTextureIndex(newPortalSRV);
        newPortal->SetTextureIndex(firstPortalSRV);

        newPortal->GetWarpPos()->SetParent(newPortalTransform);
        firstPortal->GetWarpPos()->SetParent(&firstPortal->GetTransform());
    }

    portals_.push_back(std::move(newPortal));
}