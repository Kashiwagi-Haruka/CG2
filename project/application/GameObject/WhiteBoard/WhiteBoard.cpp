#include "WhiteBoard.h"

WhiteBoard::WhiteBoard()
{
    obj_ = std::make_unique<Object3d>();
}

void WhiteBoard::Initialize()
{
    obj_->Initialize();
}

void WhiteBoard::Update()
{
    obj_->Update();
}

void WhiteBoard::Draw()
{
    obj_->Draw();
}

void WhiteBoard::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
}

void WhiteBoard::SetModel(const std::string& filePath)
{
    obj_->SetModel(filePath);
}
