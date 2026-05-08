#include "GiantGentlemanHand.h"

GiantGentlemanHand::GiantGentlemanHand()
{
    primitive_ = std::make_unique<Primitive>();
}

void GiantGentlemanHand::OnCollision(Collider* collider)
{
}

Vector3 GiantGentlemanHand::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(primitive_->GetWorldMatrix());
    /*  return primitive_->GetTransform().translate;*/
}

void GiantGentlemanHand::Initialize()
{
    primitive_->Initialize(Primitive::Box);
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer);

}

void GiantGentlemanHand::Update()
{
    Vector3 scale = primitive_->GetTransform().scale;
    scale.x *= 0.5f;
    scale.y *= 0.5f;
    scale.z *= 0.5f;
    SetAABB(AABB{ .min = {-scale.x,-scale.y,-scale.z},.max = scale });
    primitive_->Update();

}

void GiantGentlemanHand::Draw()
{
    primitive_->Draw();
}
void GiantGentlemanHand::SetCamera(Camera* camera)
{

    primitive_->SetCamera(camera);
    primitive_->UpdateCameraMatrices();

}
