#include "WhiteBoard.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"

WhiteBoard::WhiteBoard()
{
    obj_ = std::make_unique<Object3d>();
#ifdef _DEBUG
    primitive_ = std::make_unique<Primitive>();
#endif
}

void WhiteBoard::Initialize()
{
    obj_->Initialize();
#ifdef _DEBUG
    primitive_->Initialize(Primitive::Box);
    primitive_->SetColor({ 1.0f,1.0f,1.0f,0.1f });
#endif
    localAABB_ = { .min = { -0.5f,-0.5f,-0.5f},.max = {0.5f,0.5f,0.5f} };
}

void WhiteBoard::Update()
{
    obj_->Update();

    collisionTransform_ = obj_->GetTransform();

    collisionTransform_.scale = YoshidaMath::GetAABBScale(localAABB_);
    collisionTransform_.rotate = { 0.0f };
    //objectからの相対距離
    collisionTransform_.translate.y += 1.375f;

#ifdef _DEBUG
    primitive_->SetTransform(collisionTransform_);
    primitive_->Update();
#endif
}

void WhiteBoard::Draw()
{
    obj_->Draw();
#ifdef _DEBUG
    primitive_->Draw();
#endif
}

void WhiteBoard::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
#ifdef _DEBUG
    primitive_->SetCamera(camera);
#endif
}

void WhiteBoard::SetModel(const std::string& filePath)
{
    obj_->SetModel(filePath);
}

AABB WhiteBoard::GetAABB()
{
    return YoshidaMath::GetAABBWorldPos(localAABB_, collisionTransform_.translate);
}
