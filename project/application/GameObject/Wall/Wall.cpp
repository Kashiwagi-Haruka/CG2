#include "Wall.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"

Wall::Wall()
{
    primitive_ = std::make_unique<Primitive>();
    SetAABB({ .min = {-0.5f,-0.5f,-0.5f},.max = {0.5f,0.5f,0.5f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer);
}

void Wall::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {
        primitive_->SetColor({ 1.0f,0.0f,0.0f,1.0f });
    } else {
        primitive_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
    }
}

Vector3 Wall::GetWorldPosition() const
{
    return primitive_->GetTransform().translate;
}

void Wall::Update()
{
    primitive_->Update();
}

void Wall::Initialize()
{
    primitive_->Initialize(Primitive::Box);
}

void Wall::Draw()
{
    primitive_->Draw();
}


void Wall::SetCamera(Camera* camera)
{
    primitive_->SetCamera(camera);
    primitive_->UpdateCameraMatrices();
}
