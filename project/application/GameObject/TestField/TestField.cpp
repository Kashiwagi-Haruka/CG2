#include "TestField.h"
#include"Function.h"
#include"Light/CommonLight/SpotCommonLight.h"
#include"imgui.h"
TestField::TestField()
{
    box_ = std::make_unique<Primitive>();
    AABB aabb = { .min = {-50.0f,-1.0f,-50.0f},.max = {50.0f,0.0f,50.0f} };
    SetAABB(aabb);
    SetCollisionAttribute(kCollisionFloor);
    SetCollisionMask(kCollisionPlayer | kCollisionChair | kCollisionKey | kCollisionItem);
}

void TestField::Initialize()
{
    transform_ = { .scale = {28.0f,0.01f,50.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,-0.01f,0.0f} };
    box_->Initialize(Primitive::Box, "Resources/TD3_3102/2d/floor.png");
    box_->RegisterEditor("TestField");
    box_->SetTransform(transform_);
    box_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
}

void TestField::Update()
{
    box_->SetEnableLighting(true);
    box_->SetTransform(transform_);
    box_->SetUvTransform({ 50.0f,50.0f,50.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f });
    box_->Update();

#ifdef USE_IMGUI
    ImGui::Begin("TestField");
    if (ImGui::TreeNode("TestField")) {
        ImGui::DragFloat3("translate", &transform_.translate.x, 0.1f);
        ImGui::DragFloat3("rotate", &transform_.rotate.x, 0.1f);
        ImGui::DragFloat3("scale", &transform_.scale.x, 0.1f);
        ImGui::TreePop();
    }
    ImGui::End();
#endif
}

void TestField::Draw()
{

    box_->Draw();
}

void TestField::SetCamera(Camera* camera)
{
    box_->SetCamera(camera);
    box_->UpdateCameraMatrices();
}

void TestField::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {

    }
}

Vector3 TestField::GetWorldPosition() const
{
    return transform_.translate;
}
