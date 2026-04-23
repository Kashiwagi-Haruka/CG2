#include "BurningObject.h"
#include "Mesh/Object3d/Object3dCommon.h"

BurningObject::BurningObject() { obj_ = std::make_unique<Object3d>(); }
void BurningObject::Initialize() { 
	obj_->Initialize();
	obj_->SetColor({1.0f, 0.5f, 0.0f, 1.0f});
}
void BurningObject::Update() {
	obj_->Update();
}
void BurningObject::Draw() {
	Object3dCommon::GetInstance()->DrawCommon();
	obj_->Draw();
}