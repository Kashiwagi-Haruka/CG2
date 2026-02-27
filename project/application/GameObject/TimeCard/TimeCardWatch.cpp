#include "TimeCardWatch.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include<imgui.h>
#include"Camera.h"
#include"Audio.h"

namespace {
    float tMin_ = 0.0f;
    float tMax_ = 5.0f;
}

TimeCardWatch::TimeCardWatch()
{
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/timeCard", "timeCard");
    modelObj_ = std::make_unique<Object3d>();
    modelObj_->SetModel("timeCard");
    ring_ = std::make_unique<Primitive>();

}

void TimeCardWatch::Initialize()
{
    modelObj_->Initialize();
    ring_->Initialize(Primitive::Ring);
    ring_->SetColor({ 1.0f,0.0f,0.0f,1.0f });
    ring_->SetEnableLighting(false);
    transform_ = { .scale = {10.0f,10.0f,10.0f},.rotate = {-1.5f,0.0f,2.55f},.translate = {1.5f,-1.2f,0.75f} };
    lineTransform_ = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.0f,0.0f} };
}

void TimeCardWatch::SetCamera(Camera* camera)
{
    //カメラのセット
    modelObj_->SetCamera(camera);
    ring_->SetCamera(camera);
    camera_ = camera;
    assert(camera_);
}

void TimeCardWatch::Update()
{
    Matrix4x4 child = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    assert(parentTransform_);
    Matrix4x4 parent = Function::MakeAffineMatrix(parentTransform_->scale, parentTransform_->rotate, parentTransform_->translate);
    child = Function::Multiply(child, parent);

    Vector3 worldPos = YoshidaMath::GetWorldPosByMat(child);

    MakeBillboardWorldMat();
    ring_->Update();

    modelObj_->SetWorldMatrix(child);
    modelObj_->Update();


#ifdef USE_IMGUI
    ImGui::Begin("timeCardWatch");
    ImGui::DragFloat3("translate", &transform_.translate.x, 0.3f);
    ImGui::DragFloat3("scale", &transform_.scale.x, 0.3f);
    ImGui::DragFloat3("rotate", &transform_.rotate.x, 0.3f);
    ImGui::End();
#endif
}

void TimeCardWatch::Draw()
{
    modelObj_->Draw();
    ring_->Draw();
}

bool TimeCardWatch::OnCollisionObjOfMakePortal(const Ray& ray,const AABB& aabb, const Transform& transform)
{
    //ポータル作れるよ
    bool canMakePortal = YoshidaMath::RayIntersectsAABB(ray, aabb, tMin_, tMax_);

    if (canMakePortal) {
        ring_->SetColor({ 0.0f,0.0f,1.0f,1.0f });
        lineTransform_ = transform;

    } else {
        ring_->SetColor({ 1.0f,0.0f,0.0f,1.0f });

        Vector3 end = {
       .x = ray.origin.x + ray.diff.x * tMax_,
       .y = ray.origin.y + ray.diff.y * tMax_,
       .z = ray.origin.z + ray.diff.z * tMax_,
        };
        lineTransform_ = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f,0.0f,0.0f},.translate = end };
  
    }

    return canMakePortal;
}

void TimeCardWatch::MakeBillboardWorldMat()
{

    Vector3 forward = YoshidaMath::GetForward(camera_->GetWorldMatrix());
    Matrix4x4 scaleMatrix = Function::MakeScaleMatrix(lineTransform_.scale);
    Matrix4x4 translateMatrix = Function::MakeTranslateMatrix(lineTransform_.translate - forward * 0.125f);
    Matrix4x4 rotateMatrix = Function::Multiply(YoshidaMath::MakeRotateMatrix(lineTransform_.rotate), YoshidaMath::GetBillBordMatrix(camera_));
    Matrix4x4 worldMatrix = Function::Multiply(Function::Multiply(scaleMatrix, rotateMatrix), translateMatrix);

    ring_->SetWorldMatrix(worldMatrix);
}
