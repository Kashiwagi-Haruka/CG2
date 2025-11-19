#include "Goal.h"
#include "Object3d.h"
#include "GameBase.h"
#include "ModelManeger.h"
Goal::Goal() { 
	
	ModelManeger::GetInstance()->LoadModel("goal");
	object3d_ = new Object3d(); 

}
Goal::~Goal(){ 
	delete object3d_;


}
void Goal::Initialize(GameBase* gameBase){

	object3d_->Initialize(gameBase->GetObject3dCommon());
	object3d_->SetTransform(transform_);
	object3d_->Update();

}
void Goal::Update() { 
	
	
	object3d_->Update(); 

}
void Goal::Draw(){

	object3d_->Draw();

}