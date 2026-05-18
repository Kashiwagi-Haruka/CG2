#include "ElevatorNumber.h"
#include "Engine/Texture/Mesh/Object3d/Object3dCommon.h"
#include "Engine/math/Function.h"
#include <numbers>
namespace{
	const float kNumberSize = 0.1f; // エレベーターの数字のサイズ
	const float kNumberPrimitiveSize = 0.25f; // プリミティブのサイズ
    const Vector2 kNumberAspect = {3.0f, 2.0f}; // プリミティブのアスペクト比
    }
void ElevatorNumber::Initialize() {
	primitive_ = std::make_unique<Primitive>();
	primitive_->Initialize(Primitive::Plane, "Resources/TD3_3102/2d/ElevatorNumber.png");
	primitive_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	primitive_->SetEnableLighting(false);	
	transform_.scale = {kNumberAspect.x*kNumberPrimitiveSize,kNumberAspect.y*kNumberPrimitiveSize,kNumberPrimitiveSize};
	transform_.rotate = {0.0f, std::numbers::pi_v<float>, 0.0f};
	transform_.translate = {-2.0f, 3.0f, -2.0f};
}
void ElevatorNumber::Update() { 
	primitive_->SetUvTransform({kNumberSize, 1, 1}, {0, 0, 0}, {number_ * kNumberSize, 0, 0});
	primitive_->SetWorldMatrix(Function::MakeParentChildTransform(elevatorTransform_, transform_));
	primitive_->Update(); 
}
void ElevatorNumber::Draw() {
	Object3dCommon::GetInstance()->DrawCommon();
	primitive_->Draw(); 
}
void ElevatorNumber::SetCamera(Camera* camera) {
	camera_ = camera;
	primitive_->SetCamera(camera);
}