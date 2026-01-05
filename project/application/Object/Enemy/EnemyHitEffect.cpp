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
	enemyPosition_ = {0.0f, 0.0f, 0.0f};
}

void EnemyHitEffect::Activate(const Vector3& position) {
	isActive_ = true;
	activeTimer_ = activeDuration_;
	enemyPosition_ = position;
}

void EnemyHitEffect::Update() {
	if (!isActive_) {
		return;
	}

	activeTimer_ -= 1.0f / 60.0f;
	if (activeTimer_ <= 0.0f) {
		isActive_ = false;
		return;
	}

	hitTransform_.translate = enemyPosition_;

	Matrix4x4 c = camera_->GetWorldMatrix();
	c.m[3][0] = c.m[3][1] = c.m[3][2] = 0;
	Matrix4x4 world = Function::Multiply(c, Function::MakeAffineMatrix(hitTransform_.scale, hitTransform_.rotate, hitTransform_.translate));
	
	hitEffect_->SetCamera(camera_);
	hitEffect_->SetWorldMatrix(world);
	hitEffect_->Update();
}

void EnemyHitEffect::Draw() {
	if (!isActive_) {
		return;
	}

	hitEffect_->Draw();
}