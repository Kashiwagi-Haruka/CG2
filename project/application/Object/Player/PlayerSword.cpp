#include "PlayerSword.h"
#include "GameBase.h"
#include "Model/ModelManager.h"
#include "Function.h"
PlayerSword::PlayerSword() {

	ModelManager::GetInstance()->LoadModel("Resources/3d","playerSword");
	ModelManager::GetInstance()->LoadModel("Resources/3d","debugBox");
}

void PlayerSword::Initialize() {
	swordObject_ = std::make_unique<Object3d>();
	swordObject_->Initialize();
	swordObject_->SetCamera(camera);
	swordObject_->SetModel("playerSword");
#ifdef _DEBUG
	debugBox_ = std::make_unique<Object3d>();
	debugBox_->Initialize();
	debugBox_->SetCamera(camera);
	debugBox_->SetModel("debugBox");
	hitTransform_ = {
	    .scale{1, 1, 1},
        .rotate{0, 0, 0},
        .translate{0, 0, 0}
	};
#endif // _DEBUG
}

void PlayerSword::StartAttack(int comboStep) {
	isAttacking_ = true;
	currentComboStep_ = comboStep;
}

void PlayerSword::EndAttack() {
	isAttacking_ = false;
}

Vector3 PlayerSword::GetPosition() const { return hitTransform_.translate; }

void PlayerSword::Update(const Transform& playerTransform, const std::optional<Matrix4x4>& jointWorldMatrix) {

	const bool useJointAttachment = jointWorldMatrix.has_value();
	Transform swordTransform = playerTransform;
	if (useJointAttachment) {
		swordTransform = {
		    .scale{1.0f, 1.0f, 1.0f},
		    .rotate{0.0f, 0.0f, 0.0f},
		    .translate{0.0f, 0.0f, 0.0f},
		};
	} else {
		Vector3 backDir = {sinf(playerYaw_), 0.0f, cosf(playerYaw_)};
		swordTransform.translate = playerTransform.translate - backDir * distanceFromPlayer_;
	}
	Vector3 forwardDir = {sinf(playerYaw_), 0.0f, cosf(playerYaw_)};
	hitTransform_ = playerTransform;
	hitTransform_.scale = {GetHitSize(), GetHitSize(), GetHitSize()};
	hitTransform_.translate = playerTransform.translate + forwardDir * hitDistanceFromPlayer_;

	if (useJointAttachment) {
		const Matrix4x4 localMatrix = Function::MakeAffineMatrix(swordTransform.scale, swordTransform.rotate, swordTransform.translate);
		const Matrix4x4 worldMatrix = Function::Multiply(localMatrix, *jointWorldMatrix);
		swordObject_->SetWorldMatrix(worldMatrix);
	} else {
		swordObject_->SetTransform(swordTransform);
	}
	swordObject_->SetCamera(camera);
	swordObject_->Update();

#ifdef _DEBUG
	debugBox_->SetTransform(hitTransform_);
	debugBox_->SetCamera(camera);
	debugBox_->Update();
#endif // _DEBUG
}

void PlayerSword::Draw() {
	swordObject_->Draw();
#ifdef _DEBUG
	debugBox_->Draw();
#endif // _DEBUG
}
