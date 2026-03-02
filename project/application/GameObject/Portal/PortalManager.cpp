#include "PortalManager.h"
#include"Model/ModelManager.h"
#include"GameObject/TimeCard/TimeCardWatch.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include"GameObject/WhiteBoard/WalkWhiteBoard.h"

namespace {
    const constexpr uint32_t kMaxWhiteBoards = 6;
}

PortalManager::PortalManager(Vector3* pos)
{
    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/whiteBoard", "whiteBoard");
    std::unique_ptr<WalkWhiteBoard>  walkWhite = std::make_unique<WalkWhiteBoard>();
    WalkWhiteBoard::LoadAnimation("Resources/TD3_3102/3d/whiteBoard", "whiteBoard");
    walkWhite->SetModel("whiteBoard");
    walkWhite->SetTargetPosPtr(pos);
    whiteBoards_.push_back(std::move(walkWhite));

    for (int i = 0; i < kMaxWhiteBoards; ++i) {
        std::unique_ptr<WhiteBoard>  white = std::make_unique<WhiteBoard>();
        white->SetModel("whiteBoard");
        whiteBoards_.push_back(std::move(white));
    }
}

void PortalManager::Initialize()
{
    for (auto& board : whiteBoards_) {
        board->Initialize();
    }
}

void PortalManager::UpdateWhiteBoard()
{
    for (auto& board : whiteBoards_) {
        board->Update();
    }
}

void PortalManager::UpdatePortal() {

    for (auto& portal : portals_) {
        portal->Update();
    }
};

void PortalManager::DrawWhiteBoard()
{
    for (auto& board : whiteBoards_) { 
        board->Draw();
    }
}

void PortalManager::ShadowDraw()
{
    DrawWhiteBoard();

    for (auto& portal : portals_) {
        portal->SetCamera(playerCamera_->GetCamera());
        portal->UpdateCameraMatrices();
        portal->DrawRings();
        portal->DrawWarpPos();
    }
}

void PortalManager::ObjDraw()
{
    DrawWhiteBoard();

    for (auto& portal : portals_) {
        portal->SetCamera(playerCamera_->GetCamera());
        portal->UpdateCameraMatrices();
        portal->DrawPortals();
        portal->DrawRings();
        portal->DrawWarpPos();
    }
}

void PortalManager::SetPlayerCamera(PlayerCamera* camera)
{
    playerCamera_ = camera;
    for (auto& board : whiteBoards_) {
        board->SetCamera(camera->GetCamera());
    }
}

void PortalManager::CheckCollision(TimeCardWatch* timeCardWatch,const Vector3& warpPos)
{


    //whiteBoardとrayの当たり判定
    for (auto& board : whiteBoards_) {
        if (timeCardWatch->OnCollisionObjOfMakePortal(playerCamera_->GetRay(), board->GetAABB(), board->GetTransform())) {

            if (PlayerCommand::GetInstance()->Shot()) {
               //前回のポータルを削除
                portals_.clear();

                if (whiteBoard_) { 
                    whiteBoard_->SetCollisionAttribute(preCollision_);
                }

                //ショットしたらポータル作る
               std::unique_ptr  portal = std::make_unique<Portal>();
               whiteBoard_ = board.get();
               preCollision_ = whiteBoard_->GetCollisionAttribute();
               whiteBoard_->SetCollisionAttribute(kCollisionNone);

               //Portalの初期化処理
               
               portal->Initialize();

               Camera* camera = playerCamera_->GetCamera();
               portal->SetCamera(camera);
               portal->SetParentTransform(&whiteBoard_->GetTransform());
               portal->SetWarpTransform(warpPos);
               portals_.push_back(std::move(portal));


            }
            break;
        };
    }

}
