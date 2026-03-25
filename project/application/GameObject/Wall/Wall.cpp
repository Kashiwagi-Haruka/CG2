#include "Wall.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include<assert.h>

Wall::Wall()
{
    primitive_ = std::make_unique<Primitive>();
    Vector3 halfScale = Vector3{ 1.0f,1.0f,1.0f } *0.5f;
    SetAABB({ .min = -halfScale,.max = halfScale });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer|kCollisionEnemy|kCollisionKey);
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
   return YoshidaMath::GetWorldPosByMat(primitive_->GetWorldMatrix());
}

void Wall::Update()
{
    AdjustAABB();
    assert(parentMat_);
    Matrix4x4 worldMat = Function::MakeAffineMatrix(primitive_->GetTransform().scale, primitive_->GetTransform().rotate, primitive_->GetTransform().translate);
    primitive_->SetWorldMatrix(Function::Multiply(worldMat, *parentMat_));
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

void Wall::SetST(const Vector3& scale, const Vector3& translate)
{
    primitive_->SetScale(scale);
    primitive_->SetTranslate(translate);
}
