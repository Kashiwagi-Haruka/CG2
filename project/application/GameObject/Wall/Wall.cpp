#include "Wall.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"

Wall::Wall()
{

    primitive_ = std::make_unique<Primitive>();
    Vector3 halfScale = Vector3{1.0f,1.0f,1.0f} *0.5f;
    SetAABB({ .min = -halfScale,.max = halfScale });
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
    AdjustAABB();
    primitive_->Update();
}

void Wall::Initialize()
{
    primitive_->Initialize(Primitive::Box, "Resources/TD3_3102/2d/wall.png");
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

void Wall::AdjustAABB()
{
    Vector3 halfScale = primitive_->GetTransform().scale * 0.5f;
    SetAABB({ .min = -halfScale,.max = halfScale });

}
