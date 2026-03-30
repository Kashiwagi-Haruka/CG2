#include "WhiteBoardManager.h"

#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "GameObject/KeyBindConfig.h"
#include "GameObject/SEManager/SEManager.h"
#include "GameObject/WhiteBoard/WalkWhiteBoard.h"
#include "Model/ModelManager.h"

namespace {
    const constexpr uint32_t kMaxWhiteBoards = 5;
}

WhiteBoardManager::WhiteBoardManager(Vector3* playerPos) {
    playerPos_ = playerPos;

    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/whiteBoard", "whiteBoard");

    std::unique_ptr<WalkWhiteBoard> walkWhite = std::make_unique<WalkWhiteBoard>();
    WalkWhiteBoard::LoadAnimation("Resources/TD3_3102/3d/whiteBoard", "whiteBoard");
    walkWhite->SetModel("whiteBoard");
    walkWhite->SetTargetPosPtr(playerPos_);
    whiteBoards_.push_back(std::move(walkWhite));

    for (int i = 0; i < kMaxWhiteBoards; ++i) {
        std::unique_ptr<WhiteBoard> white = std::make_unique<WhiteBoard>();
        white->SetModel("whiteBoard");
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
    canMakePortal_ = false;
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

    canMakePortal_ = false;

    for (auto& board : whiteBoards_) {
        if (!OnCollisionRay(playerCamera, board->GetAABB(), board->GetCollisionTransform().translate)) {
            continue;
        }

        if (board->GetCollisionAttribute() == kCollisionNone) {
            continue;
        }

        canMakePortal_ = true;

        if (!PlayerCommand::GetInstance()->Shot()) {
            break;
        }

        // ショットSE鳴らす
        SEManager::SoundPlay(SEManager::SHOT);

        if (preWhiteBoards_.size() >= 2) {
            // ポータルの生成が2個以上になったら
            preWhiteBoards_.at(0)->ResetCollisionAttribute();
            preWhiteBoards_.erase(preWhiteBoards_.begin());
        }

        preWhiteBoards_.push_back(board.get());

        // マスクの設定とフラグの初期化
        preWhiteBoards_.back()->SetCollisionAttributeNoneAndInitialize();

        return preWhiteBoards_.back();
    }

    return nullptr;
}