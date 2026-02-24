#include "Portal.h"
#include"Function.h"
#include"application/GameObject/YoshidaMath/YoshidaMath.h"
#include<cassert>

//音楽
SoundData Portal::warpSE_;

void Portal::LoadSE()
{
    warpSE_ = Audio::GetInstance()->SoundLoadFile("Resources/audio/SE/magic.mp3");
    Audio::GetInstance()->SetSoundVolume(&warpSE_, 1.0f);
}

void Portal::UnLoadSE()
{
    Audio::GetInstance()->SoundUnload(&warpSE_);
}

void Portal::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {
        Audio::GetInstance()->SoundPlayWave(warpSE_, false);
    }
}

Vector3 Portal::GetWorldPosition() const
{
    return transform_.translate;
}

Portal::Portal()
{
    transform_ = { .scale = {1.5f,1.5f,1.5f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.75f,0.0f} };
    sphere_ = { .center = {transform_.translate},.radius = 0.5f };

    SetRadius(sphere_.radius);
    SetCollisionAttribute(kCollisionPortal);
    SetCollisionMask(kCollisionPlayer);

    ring_ = std::make_unique<Primitive>();

    //ワープ座標
    warpPos_ = std::make_unique<WarpPos1>();
}

Portal::~Portal()
{
   
}

void Portal::Initialize()
{

    transform_ = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.0f,0.0f} };
    ring_->Initialize(Primitive::Ring, "Resources/TD3_3102/2d/atHome.jpg");
    ring_->SetTransform(transform_);
    ring_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
    //ライティングしない
    ring_->SetEnableLighting(false);
    uvTranslateY_ = 0.0f;
    uvMat_ = Function::MakeTranslateMatrix({ 0.0f,uvTranslateY_ ,0.0f });
    sphere_ = { .center = {transform_.translate},.radius = 0.5f };
    //ワープ座標
    warpPos_->Initialize();

}

void Portal::Update()
{
    uvTranslateY_ += YoshidaMath::kDeltaTime;
    uvMat_ = Function::MakeTranslateMatrix({ 0.0f,uvTranslateY_ ,0.0f });
    ring_->SetUvTransform(uvMat_);

    ring_->Update();
    //ワープ地点
    warpPos_->Update();


}

void Portal::Draw()
{
    ring_->Draw();
    //ワープ地点
    warpPos_->Draw();
}

void Portal::SetCamera(Camera* camera)
{
    ring_->SetCamera(camera);
    //ワープ地点
    warpPos_->SetCamera(camera);
}

void Portal::SetRingWorldMatrix(Camera* camera)
{
    Vector3 forward = YoshidaMath::GetForward(camera->GetWorldMatrix());

    Matrix4x4 scaleMatrix = Function::MakeScaleMatrix(transform_.scale);
    Matrix4x4 translateMatrix = Function::MakeTranslateMatrix(transform_.translate - forward*0.125f);
    Matrix4x4 rotateMatrix = Function::Multiply(YoshidaMath::MakeRotateMatrix(transform_.rotate), YoshidaMath::GetBillBordMatrix(camera));
    Matrix4x4 worldMatrix = Function::Multiply(Function::Multiply(scaleMatrix, rotateMatrix), translateMatrix);

    ring_->SetWorldMatrix(worldMatrix);
}

const Sphere& Portal::GetSphere()
{
    sphere_ = { .center = {transform_.translate},.radius = 0.5f };
    return sphere_;
}
