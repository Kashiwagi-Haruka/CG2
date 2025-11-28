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
void Goal::Initialize(Camera* camera){
	camera_ = camera;
	object3d_->SetCamera(camera_);
	object3d_->SetModel("goal");
	object3d_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	transform_ = {
	    {1, 1, 1},
        {0, 0, 0},
        {5, 2, 0}
    };
	object3d_->SetTransform(transform_);
	
	object3d_->Update();

}
void Goal::Update() { 
	
	object3d_->SetCamera(camera_);
	object3d_->Update(); 

}
void Goal::Draw(){

	object3d_->Draw();

}