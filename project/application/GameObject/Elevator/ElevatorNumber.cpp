#include "ElevatorNumber.h"
#include "Engine/Texture/Mesh/Object3d/Object3dCommon.h"
void ElevatorNumber::Initialize() {
	primitive_ = std::make_unique<Primitive>();
	primitive_->Initialize(Primitive::Plane, "Resources/TD3_3102/2d/ElevatorNumber.png");
	primitive_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	
}
void ElevatorNumber::Update() { 
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