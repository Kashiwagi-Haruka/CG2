#include "WhiteBoard.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"Function.h"

WhiteBoard::WhiteBoard()
{
    obj_ = std::make_unique<Object3d>();
#ifdef _DEBUG
    primitive_ = std::make_unique<Primitive>();
#endif
}

void WhiteBoard::OnCollision(Collider* collider)
{
   
}

Vector3 WhiteBoard::GetWorldPosition() const
{
    return obj_->GetTransform().translate;
}

void WhiteBoard::Initialize()
{
    obj_->Initialize();
    obj_->SetRotate({ 0.0f,Function::kPi,0.0f });
#ifdef _DEBUG
    primitive_->Initialize(Primitive::Box);
    primitive_->SetColor({ 1.0f,1.0f,1.0f,0.1f });
#endif
    localAABB_ = { .min = { -0.5f,-0.5f,-0.5f},.max = {0.5f,0.5f,0.5f} };

    /* SetRadius(1.0f);*/
    SetAABB(AABB{ .min = {-0.5f,0.0f,-0.5f}, .max = {0.5f,1.5f,0.5f} });
    SetCollisionAttribute(kCollisionFloor);
    SetCollisionMask(kCollisionPlayer);
}

void WhiteBoard::Update()
{
    obj_->Update();

    obj_->SetShininess(80.0f);
    collisionTransform_ = obj_->GetTransform();
    collisionTransform_.scale = YoshidaMath::GetAABBScale(localAABB_);

    //objectからの相対距離
    collisionTransform_.translate.y += 1.375f;

#ifdef _DEBUG
    primitive_->SetTransform(collisionTransform_);
    primitive_->Update();
#endif
}

void WhiteBoard::Draw() {

    obj_->Draw();
#ifdef _DEBUG
    primitive_->Draw();
#endif
}

void WhiteBoard::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();

#ifdef _DEBUG
    primitive_->SetCamera(camera);
    primitive_->UpdateCameraMatrices();
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

void WhiteBoard::ResetCollisionAttribute()
{
    SetCollisionAttribute(kCollisionFloor);
}
