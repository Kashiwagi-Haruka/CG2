#include "Portal.h"
#include"Function.h"
#include"application/GameObject/YoshidaMath/YoshidaMath.h"
#include<cassert>
#include"GameObject/YoshidaMath/Easing.h"
#include "Object3d/Object3dCommon.h"
#include "DirectXCommon.h"
#include<algorithm>
#include"Camera.h"

Camera* Portal::sceneCamera_ = nullptr;
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
    if (!isPlayerHit_) {
        if (collider->GetCollisionAttribute() == kCollisionPlayer) {
            Audio::GetInstance()->SoundPlayWave(warpSE_, false);
            isPlayerHit_ = true;

        }
    }
}

Vector3 Portal::GetWorldPosition() const
{
    return transform_.translate;
}

Portal::Portal()
{
    sphere_ = { .center = {transform_.translate},.radius = 0.0625f*0.25f };
    SetRadius(sphere_.radius);
    SetCollisionAttribute(kCollisionPortal);
    SetCollisionMask(kCollisionPlayer);

    ring_ = std::make_unique<Primitive>();
    //ワープ座標
    warpPos_ = std::make_unique<WarpPos>();
}

Portal::~Portal()
{
    warpPos_.reset();
    ring_.reset();
    portalCircle_.reset();
    portalRenderTexture_.reset();

}

void Portal::Initialize()
{
    isPlayerHit_ = false;
    scaleTimer_ = 0.0f;
    uvRotateZ_ = 0.0f;
    transform_ = { .scale = {0.0f,0.0f,0.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.0f,0.0f} };
    ring_->Initialize(Primitive::Ring, "Resources/TD3_3102/2d/ring.png", 128);
    //ライティングしない
    ring_->SetEnableLighting(false);
    ring_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
    //ワープ座標
    warpPos_->Initialize();

    portalRenderTexture_ = std::make_unique<RenderTexture2D>();
    portalRenderTexture_->Initialize(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, { 0.05f, 0.05f, 0.1f, 1.0f });
    portalCircle_ = std::make_unique<PortalMesh>();
    portalCircle_->Initialize("Resources/TD3_3102/2d/atHome.jpg");
    //テクスチャここで設定するよーん
    portalCircle_->SetTextureIndex(portalRenderTexture_->GetSrvIndex());
    preRotY_ = { 0.0f };
}

void Portal::Update()
{
    isPlayerHit_ = false;

    uvRotateZ_ += YoshidaMath::kDeltaTime * 2.0f;
    ring_->SetUvTransform(Vector3(1, 1, 1), Vector3(0, 0, uvRotateZ_), Vector3(0, 0, 0), Vector2(0.5f, 0.5f));
    UpdatePortalWorldMatrix();
    ring_->Update();

    portalCircle_->Update();
    //ワープ地点
    warpPos_->Update();
}

void Portal::DrawPortals() {
    if (portalCircle_) {
        portalCircle_->Draw();
    }
}

void Portal::DrawRings() {
    //Object3dCommon::GetInstance()->DrawCommonNoCull();
    ring_->Draw();
}


void Portal::BeginRender()
{
    portalRenderTexture_->BeginRender();

}

void Portal::TransitionToShaderResource()
{
    portalRenderTexture_->TransitionToShaderResource();
    Object3dCommon::GetInstance()->GetDxCommon()->ExecuteCommandListAndWait();
}

void Portal::SetCamera(Camera* camera)
{
    sceneCamera_ = camera;
    portalCircle_->SetObjectCamera(camera);

    ring_->SetCamera(camera);
    ring_->UpdateCameraMatrices();
    warpPos_->SetCamera(camera);

}

void Portal::SetPortalWorldMatrix()
{
    SetParentTransformToTransform();
    Vector3 forward = SetSceneCameraAndParentAndGetForward();
    //回転を方向別でセットする

    SetRotateFromDirection(forward);
    SetTranslate(forward);
    UpdateWorldMatrix();
}

void Portal::UpdatePortalWorldMatrix()
{
    Vector3 forward = SetSceneCameraAndParentAndGetForward();
    SetTranslate(forward);
    transform_.rotate.y = preRotY_+ parentTransform->rotate.y;
    UpdateScale();
    UpdateWorldMatrix();
}

void Portal::SetRotateFromDirection(const Vector3& forward)
{
    Vector3 direction = YoshidaMath::GetDirectionFromRotateY(parentTransform->rotate.y);
    float dot = Function::Dot(forward, direction);

    if (dot < 0.0f) {
        //向き合っている
        ring_->SetColor({ 1.0f,0.0f,0.0f,1.0f });
        preRotY_ = Function::kPi;
    } else {
        ring_->SetColor({ 0.0f,0.0f,1.0f,1.0f });
        preRotY_ =0.0f;
    }
    transform_.rotate.y = preRotY_ + parentTransform->rotate.y;
}

void Portal::UpdateScale()
{
    scaleTimer_ += YoshidaMath::kDeltaTime;
    scaleTimer_ = std::clamp(scaleTimer_, 0.0f, 1.0f);
    transform_.scale = YoshidaMath::Easing::EaseInOutBack({ 0.0f,0.0f,0.0f }, parentTransform->scale, scaleTimer_);
}

void Portal::UpdateWorldMatrix()
{
    Matrix4x4  worldMatrix = Function::MakeAffineMatrix(transform_.scale*0.9f, transform_.rotate, transform_.translate);
    Matrix4x4  worldMatrix1 = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, ringTranslate_);
    portalCircle_->SetWorldMatrix(worldMatrix);
    ring_->SetWorldMatrix(worldMatrix1);
}

void Portal::SetTranslate(const Vector3& forward)
{
    transform_.translate = parentTransform->translate - forward * 0.0625f;
    ringTranslate_ = transform_.translate - forward * 0.0625f * 0.125f;
}

Vector3 Portal::SetSceneCameraAndParentAndGetForward()
{
    assert(sceneCamera_);
    return YoshidaMath::GetForward(sceneCamera_->GetWorldMatrix());
}

void Portal::SetParentTransformToTransform()
{
    assert(parentTransform);
    transform_ = *parentTransform;
}
