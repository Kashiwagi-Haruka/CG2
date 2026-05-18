#include "ObjectCollider.h"
#include"Object3d/Object3dCommon.h"
#include"Function.h"

ObjectCollider::ObjectCollider()
{
    primitive_ = std::make_unique<Primitive>();
}

void ObjectCollider::OnCollision(Collider* collider)
{
}

Vector3 ObjectCollider::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(primitive_->GetWorldMatrix());
}


void ObjectCollider::Update()
{
    Vector3 scale = primitive_->GetTransform().scale;

    auto type = GetType();

    if (type == YoshidaMath::ColliderType::kAABB) {
        scale.x *= 0.5f;
        scale.y *= 0.5f;
        scale.z *= 0.5f;
        SetAABB(AABB{ .min = {-scale.x,-scale.y,-scale.z},.max = scale });
    } else if (type == YoshidaMath::ColliderType::kSphere) {
        SetRadius(scale.x * 0.5f);
    } else {
        //後で書く
    }

    auto transform = primitive_->GetTransform();
    if (parent_) {
        Matrix4x4 child = Function::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
        child = Function::Multiply(child, *parent_);
        primitive_->SetWorldMatrix(child);
    } else {
        primitive_->SetTransform(transform);
    }

    primitive_->Update();

}

void ObjectCollider::Draw()
{
    Object3dCommon::GetInstance()->DrawCommon();
    primitive_->Draw();
}
void ObjectCollider::SetCamera(Camera* camera)
{

    primitive_->SetCamera(camera);
    primitive_->UpdateCameraMatrices();

}

void ObjectCollider::Initialize(const YoshidaMath::ColliderType& type)
{
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer);

    if (type == YoshidaMath::ColliderType::kAABB) {
        primitive_->Initialize(Primitive::Box);
        SetAABB({ .min = {-0.5f,-0.5f,-0.5f},.max = {0.5f,0.5f,0.5f} });
    } else if (type == YoshidaMath::ColliderType::kSphere) {
        SetRadius(primitive_->GetTransform().scale.x * 0.5f);
    } else {
        //BOXを入れる
        primitive_->Initialize(Primitive::Box);
    }

}
