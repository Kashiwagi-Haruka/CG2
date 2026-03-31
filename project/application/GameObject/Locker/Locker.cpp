#include "Locker.h"

#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/Player/Player.h"
#include"Object3d/Object3dCommon.h"
PlayerCamera* Locker::playerCamera_ = nullptr;

Locker::Locker()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Locker", "Locker");
    obj_->SetModel("Locker");
    SetAABB({ .min = {-0.5f,0.0f,-0.5f},.max = {0.5f,1.8f,0.5f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer| kCollisionKey);
}

void Locker::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {

    }
}

Vector3 Locker::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}

void Locker::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}

void Locker::Update()
{

    CheckCollision();
    obj_->Update();
}

void Locker::Initialize()
{
    isRayHit_ = false;
    obj_->Initialize();
}

void Locker::Draw()
{
    obj_->Draw();
}


void Locker::CheckCollision()
{
    isRayHit_ = OnCollisionRay();
    if (isRayHit_) {
        //rayの当たり判定

        if (PlayerCommand::GetInstance()->InteractTrigger()) {
       
            if (!PlayerCommand::GetIsGrab()) {

            }
        }
    }
 
}


bool Locker::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(GetAABB(), GetWorldPosition());
}
