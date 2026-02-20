#include "TestField.h"

TestField::TestField()
{
    plane_ = std::make_unique<Primitive>();
}

void TestField::Initialize()
{
    transform_ = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f},.translate = {0.0f,0.0f,0.0f} };
    plane_->Initialize(Primitive::Plane, "Resources/TD3_3102/2d/atHome.jpg");
    plane_->SetTransform(transform_);
}

void TestField::Update()
{
    plane_->Update();
}

void TestField::Draw()
{
    plane_->Draw();
}

void TestField::SetCamera(Camera* camera)
{
    plane_->SetCamera(camera);
}
