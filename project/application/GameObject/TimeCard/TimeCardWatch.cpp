#include "TimeCardWatch.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include<imgui.h>
#include"GameObject/KeyBindConfig.h"
namespace {
    float tMin_ = 0.1f;
    float rayDiff = 10.0f;
}

TimeCardWatch::TimeCardWatch()
{
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/timeCard", "timeCard");
    modelObj_ = std::make_unique<Object3d>();
    modelObj_->SetModel("timeCard");
    line_ = std::make_unique<Primitive>();
    ray_ = { .origin = {0.0f},.diff = {0.0f} };
}

void TimeCardWatch::Initialize()
{
    modelObj_->Initialize();
    line_->Initialize(Primitive::Box);
    line_->SetColor({ 1.0f,0.0f,0.0f,1.0f });
    transform_ = { .scale = {10.0f,10.0f,10.0f},.rotate = {-1.5f,0.0f,2.55f},.translate = {1.5f,-1.2f,0.75f} };
    lineTransform_ = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.0f,0.0f} };
}

void TimeCardWatch::SetCamera(Camera* camera)
{
    //カメラのセット
    modelObj_->SetCamera(camera);
    line_->SetCamera(camera);
}

void TimeCardWatch::Update()
{
    Matrix4x4 child = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    assert(parentTransform_);
    Matrix4x4 parent = Function::MakeAffineMatrix(parentTransform_->scale, parentTransform_->rotate, parentTransform_->translate);
    child = Function::Multiply(child, parent);

    Vector3 worldPos = YoshidaMath::GetWorldPosByMat(child);

    Vector3 end = {
     .x = ray_.origin.x + ray_.diff.x * rayDiff,
     .y = ray_.origin.y + ray_.diff.y * rayDiff,
     .z = ray_.origin.z + ray_.diff.z * rayDiff,
    };

    lineTransform_.translate = end;
    line_->SetTransform(lineTransform_);
    /*   line_->SetLinePositions(worldPos, end);*/
    line_->Update();

    modelObj_->SetWorldMatrix(child);
    modelObj_->Update();


#ifdef USE_IMGUI
    ImGui::Begin("timeCardWatch");
    ImGui::DragFloat3("translate", &transform_.translate.x, 0.3f);
    ImGui::DragFloat3("scale", &transform_.scale.x, 0.3f);
    ImGui::DragFloat3("rotate", &transform_.rotate.x, 0.3f);

    ImGui::DragFloat3("origin", &ray_.origin.x, 0.3f);
    ImGui::DragFloat3("diff", &ray_.diff.x, 0.3f);
    ImGui::DragFloat("rayDiff", &rayDiff, 0.3f);
    ImGui::End();
#endif
}

void TimeCardWatch::Draw()
{
    modelObj_->Draw();
    line_->Draw();
}

void TimeCardWatch::SetRay(const Vector3& origin, const Vector3& diff)
{
    ray_.origin = origin;
    ray_.diff = diff;
}

void TimeCardWatch::OnCollisionObjOfMakePortal(const AABB& aabb)
{
    //ポータル作れるよ
    bool canMakePortal = YoshidaMath::RayIntersectsAABB(ray_, aabb, tMin_, rayDiff);

    if (canMakePortal) {

        if (PlayerCommand::GetInstance()->Shot()) {
            //ショットしたら

            //ポータル作る
        }

    };
}
