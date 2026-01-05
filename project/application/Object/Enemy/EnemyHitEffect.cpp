#include "EnemyHitEffect.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include <numbers>
void EnemyHitEffect::Initialize() {

	ModelManeger::GetInstance()->LoadModel("HitEffect");

	hitEffect_ = std::make_unique<Object3d>();
	hitEffect_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	hitEffect_->SetModel("HitEffect");
	hitEffect_->SetCamera(camera_);
	hitTransform_ = {
	    .scale{1, 1, 1},
        .rotate{0, 0, 0},
        .translate{0, 0, 0}
    };
}

void EnemyHitEffect::Update(){
	hitTransform_.translate = enemyTransform_.translate;

	Matrix4x4 c = camera_->GetWorldMatrix();
	c.m[3][0] = c.m[3][1] = c.m[3][2] = 0;
	Matrix4x4 world = Function::Multiply(c, Function::MakeAffineMatrix(hitTransform_.scale, hitTransform_.rotate, hitTransform_.translate));
	
	hitEffect_->SetCamera(camera_);
	hitEffect_->SetWorldMatrix(world);
	hitEffect_->Update();
}

void EnemyHitEffect::Draw() { 
	hitEffect_->Draw(); 
}