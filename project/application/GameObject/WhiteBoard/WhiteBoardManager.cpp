#include "WhiteBoardManager.h"
#include "Mesh/Object3d/Object3dCommon.h"
#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "GameObject/KeyBindConfig.h"
#include "GameObject/SEManager/SEManager.h"
#include "GameObject/WhiteBoard/WalkWhiteBoard.h"
#include "Model/ModelManager.h"
#include <string>

WhiteBoardManager::WhiteBoardManager(Vector3* playerPos, const uint32_t createWhiteBoardNum, const uint32_t createWalkWhiteBoardNum) {
    playerPos_ = playerPos;

    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/whiteBoard", "whiteBoard");

    WalkWhiteBoard::LoadAnimation("Resources/TD3_3102/3d/whiteBoard", "whiteBoard");

    for (uint32_t i = 0; i < createWalkWhiteBoardNum; ++i) {
        std::unique_ptr<WalkWhiteBoard> walkWhite = std::make_unique<WalkWhiteBoard>();
        walkWhite->SetModel("whiteBoard");
        walkWhite->SetEditorRegistrationName("WalkWhiteBoard" + std::to_string(i));
        walkWhite->SetTargetPosPtr(playerPos_);
        whiteBoards_.push_back(std::move(walkWhite));
    }

    for (uint32_t i = 0; i < createWhiteBoardNum; ++i) {
        std::unique_ptr<WhiteBoard> white = std::make_unique<WhiteBoard>();
        white->SetModel("whiteBoard");
        white->SetEditorRegistrationName("WhiteBoard" + std::to_string(i));
        whiteBoards_.push_back(std::move(white));
    }
}

WhiteBoardManager::~WhiteBoardManager() {
    for (auto& board : whiteBoards_) {
        board.reset();
    }
    whiteBoards_.clear();
}

void WhiteBoardManager::Initialize() {
    for (auto& board : whiteBoards_) {
        board->Initialize();
    }

    preWhiteBoards_.clear();
}

void WhiteBoardManager::Update() {
    for (auto& board : whiteBoards_) {
        board->Update();
    }
}

void WhiteBoardManager::Draw() {

    for (auto& board : whiteBoards_) {
        board->Draw();
    }
}

void WhiteBoardManager::SetCamera(Camera* camera) {
    for (auto& board : whiteBoards_) {
        board->SetCamera(camera);
    }
}

bool WhiteBoardManager::OnCollisionRay(PlayerCamera* playerCamera, const AABB& aabb, const Vector3& pos) { return playerCamera->OnCollisionRay(aabb, pos); }

WhiteBoard* WhiteBoardManager::CheckCollision(PlayerCamera* playerCamera) {
   
    if (!playerCamera) {
        return nullptr;
    }

    for (auto& board : whiteBoards_) {

        if (!OnCollisionRay(playerCamera, board->GetAABB(), board->GetCollisionTransform().translate)) {
            continue;
        }

        //ポータルとカメラが向き合っているかどうか
        if (!board->IsFacingSurface(playerCamera->GetCamera()->GetWorldMatrix())) {
            continue;
        }

        if (board->GetCollisionAttribute() == kCollisionNone) {
            continue;
        }

        //ボード返すよーん
        return board.get();
    }

    return nullptr;
}

void WhiteBoardManager::SetPortal(WhiteBoard* board)
{

    if (preWhiteBoards_.size() >= 2) {
        // ポータルの生成が2個以上になったら
        preWhiteBoards_.at(0)->ResetCollisionAttribute();
        preWhiteBoards_.erase(preWhiteBoards_.begin());
    }

    preWhiteBoards_.push_back(board);

    // マスクの設定とフラグの初期化
    preWhiteBoards_.back()->SetCollisionAttributeNoneAndInitialize();

}
