#include "FileObject.h"
#include"Model/ModelManager.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"


FileObject::FileObject()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/file", "file");
    obj_->SetModel("file");
    SetAABB({ .min = {-0.5f,0.0f,-0.5f},.max = {0.5f,0.3f,0.5f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer);
}

void FileObject::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {

    }
}

Vector3 FileObject::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}

void FileObject::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}

void FileObject::Update()
{
    obj_->Update();
}

void FileObject::Initialize()
{
    obj_->Initialize();
    obj_->RegisterEditor(editorRegistrationName_);
}

void FileObject::Draw()
{

    obj_->Draw();

}

