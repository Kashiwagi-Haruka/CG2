#include "TestField.h"
#include"Function.h"
#include"Light/CommonLight/SpotCommonLight.h"
#include"imgui.h"
TestField::TestField()
{
    box_ = std::make_unique<Primitive>();
    AABB aabb = { .min = {-14.0f,-1.0f,-12.5f},.max = {14.0f,0.0f,12.5f} };
    SetAABB(aabb);
    SetCollisionAttribute(kCollisionFloor);
    SetCollisionMask(kCollisionPlayer | kCollisionChair | kCollisionKey | kCollisionItem);
}

void TestField::Initialize()
{

    box_->Initialize(Primitive::Box, "Resources/TD3_3102/2d/floor.png");
    box_->RegisterEditor("TestField");
    box_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
}

void TestField::Update()
{

    SetCollisionAttribute(kCollisionFloor);

    box_->SetEnableLighting(true);
    box_->SetUvTransform({ 50.0f,50.0f,50.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f });
    box_->Update();
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
    return box_->GetTransform().translate;
}
