#include "GiantGentlemanHead.h"

GiantGentlemanHead::GiantGentlemanHead()
{
    primitive_ = std::make_unique<Primitive>();
}

void GiantGentlemanHead::OnCollision(Collider* collider)
{
}

Vector3 GiantGentlemanHead::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(primitive_->GetWorldMatrix());
}

void GiantGentlemanHead::Initialize()
{
    primitive_->Initialize(Primitive::Sphere);
    primitive_->RegisterEditor("GiantGentlemanHead");
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer);

}

void GiantGentlemanHead::Update()
{
    SetRadius(primitive_->GetTransform().scale.x*0.5f);
    primitive_->Update();
  
}

void GiantGentlemanHead::Draw()
{
    primitive_->Draw();
}
void GiantGentlemanHead::SetCamera(Camera* camera)
{

    primitive_->SetCamera(camera);
    primitive_->UpdateCameraMatrices();

}
