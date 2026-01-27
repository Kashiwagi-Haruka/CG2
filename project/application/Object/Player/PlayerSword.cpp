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
#endif // _DEBUG
}

void PlayerSword::StartAttack(int comboStep) {
	isAttacking_ = true;
	attackTimer_ = 0.0f;
	currentComboStep_ = comboStep;

	// コンボ段階ごとに攻撃時間を設定
	switch (comboStep) {
	case 1:
		attackDuration_ = 0.25f; // 1段目: 速い
		break;
	case 2:
		attackDuration_ = 0.28f; // 2段目: やや速い
		break;
	case 3:
		attackDuration_ = 0.32f; // 3段目: 普通
		break;
	case 4:
		attackDuration_ = 0.45f; // 4段目: フィニッシュで長め
		break;
	case 5:
		attackDuration_ = 0.6f; // 落下攻撃: 長め
		break;
	case 6:
		attackDuration_ = 0.5f; // 重撃: 強力で長め
		break;
	default:
		attackDuration_ = 0.3f;
		break;
	}
}

void PlayerSword::EndAttack() {
	isAttacking_ = false;
	attackTimer_ = 0.0f;
}

Vector3 PlayerSword::GetPosition() const { return swordObject_->GetTransform().translate; }

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

	// 攻撃中は振る
	if (isAttacking_) {
		attackTimer_ += 1.0f / 60.0f;
		// 攻撃終了判定
		if (attackTimer_ >= attackDuration_) {
			isAttacking_ = false;
			attackTimer_ = 0.0f;
		}
	}

	
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
	if (useJointAttachment) {
		const Matrix4x4 localMatrix = Function::MakeAffineMatrix(swordTransform.scale, swordTransform.rotate, swordTransform.translate);
		const Matrix4x4 worldMatrix = Function::Multiply(localMatrix, *jointWorldMatrix);
		debugBox_->SetWorldMatrix(worldMatrix);
	} else {
		debugBox_->SetTransform(swordTransform);
	}
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
