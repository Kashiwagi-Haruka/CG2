#include "AutoLockSystem.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"cassert"

AutoLockSystem::AutoLockSystem()
{
    obj_ = std::make_unique<Primitive>();
    SetAABB({ .min = {-0.5f,0.0f,-0.375f},.max = {0.5f,0.02f,0.375f} });
    SetCollisionAttribute(kCollisionMat);
    SetCollisionMask(kCollisionPlayer);
}

void AutoLockSystem::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {
        //プレイヤーと当たった時
        isPlayerHit_ = true;
    }
}

Vector3 AutoLockSystem::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}

void AutoLockSystem::Update()
{
    isPlayerHit_ = false;
    assert(parentMat_);
    obj_->SetTranslate({ 0.0f,0.0f,-0.75f });
    obj_->SetScale(YoshidaMath::GetAABBScale(GetAABB()));

    Transform transform = obj_->GetTransform();
    Matrix4x4 worldMat = Function::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
    worldMat = Function::Multiply(worldMat, *parentMat_);
    obj_->SetWorldMatrix(worldMat);
    obj_->Update();
}

void AutoLockSystem::Initialize()
{
    isPlayerHit_ = false;
    obj_->Initialize(Primitive::Box,"Resources/TD3_3102/2d/floor.png");

}

void AutoLockSystem::Draw()
{
    obj_->Draw();
}

void AutoLockSystem::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}
