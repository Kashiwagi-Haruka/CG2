#include "TestField.h"
#include"Function.h"
TestField::TestField()
{
    plane_ = std::make_unique<Primitive>();
}

void TestField::Initialize()
{
    transform_ = { .scale = {100.0f,100.0f,100.0f},.rotate = {Function::kPi * 0.5f,0.0f,0.0f},.translate = {0.0f,0.0f,0.0f} };
    plane_->Initialize(Primitive::Plane, "Resources/TD3_3102/2d/atHome.jpg");
    plane_->SetTransform(transform_);
    plane_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
    plane_->SetEnableLighting(false);
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
