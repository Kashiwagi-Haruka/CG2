#include "WhiteBoardManager.h"
#include"Model/ModelManager.h"

namespace {
    const constexpr uint32_t kMaxWhiteBoards = 6;
}

WhiteBoardManager::WhiteBoardManager()
{

    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/whiteBoard", "whiteBoard");
    for (int i = 0; i < kMaxWhiteBoards; ++i) {
        std::unique_ptr<WhiteBoard>  white = std::make_unique<WhiteBoard>(); 
        white->SetModel("whiteBoard");
        whiteBoards_.push_back(std::move(white));
    }
}

void WhiteBoardManager::Initialize()
{
    for (auto& board : whiteBoards_) {
        board->Initialize();
    }
}

void WhiteBoardManager::Update()
{
    for (auto& board : whiteBoards_) {
        board->Update();
    }
}

void WhiteBoardManager::Draw()
{
    for (auto& board : whiteBoards_) {
        board->Draw();
    }
}

void WhiteBoardManager::SetCamera(Camera* camera)
{
    for (auto& board : whiteBoards_) {
        board->SetCamera(camera);
    }
}
