#include "PortalManager.h"
#include"Model/ModelManager.h"
#include"GameObject/TimeCard/TimeCardWatch.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
namespace {
    const constexpr uint32_t kMaxWhiteBoards = 6;
}

PortalManager::PortalManager()
{
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/whiteBoard", "whiteBoard");
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

void PortalManager::Update()
{
    for (auto& board : whiteBoards_) {
        board->Update();
    }

    for (auto& portal : portals_) {
        portal->Update();
    }
}

void PortalManager::Draw()
{
    for (auto& board : whiteBoards_) {
        board->Draw();
    }

    for (auto& portal : portals_) {
        portal->Draw();
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

                //ショットしたらポータル作る
               std::unique_ptr  portal = std::make_unique<Portal>();
               
               //Portalの初期化処理
               portal->Initialize();
               Camera* camera = playerCamera_->GetCamera();
               portal->SetCamera(camera);
               portal->SetTransform(board->GetTransform());
               portal->SetRingWorldMatrix(camera);
               portal->SetWarpPos(warpPos);
               
               portals_.push_back(std::move(portal));


            }
            break;
        };
    }

}
