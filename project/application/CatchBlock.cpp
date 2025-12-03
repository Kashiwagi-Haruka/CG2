#include "CatchBlock.h"
#include "ModelManeger.h"
#include "GameBase.h"
CatchBlock::CatchBlock(){

	object3d_ = new Object3d();
	ModelManeger::GetInstance()->LoadModel("Catch_Block");
}

CatchBlock::~CatchBlock(){

	delete object3d_;

}
void CatchBlock::SetCamera(Camera* camera) {

camera_ = camera;

}
void CatchBlock::Initialize() {
	object3d_->SetModel("Catch_Block");
	transform_ = {
	    .scale = {1, 1, 1},
          .rotate = {0, 0, 0},
          .translate = {5, 5, 0}
    };
	object3d_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	object3d_->SetTransform(transform_);
	object3d_->Update();

}

void CatchBlock::Update(){




	object3d_->Update();
}

void CatchBlock::Draw() { 
	
	
	object3d_->Draw(); 


}