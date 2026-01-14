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

void PlayerSword::Update(const Transform& playerTransform) {

	
	Vector3 backDir = {sinf(playerYaw_), 0.0f, cosf(playerYaw_)};
	Transform swordTransform = playerTransform;
	swordTransform.translate = playerTransform.translate - backDir * distanceFromPlayer_;

	// 攻撃中は振る
	if (isAttacking_) {
		attackTimer_ += 1.0f / 60.0f;

		// コンボ段階ごとに異なるモーション
		switch (currentComboStep_) {
		case 1: // 1段目: 横薙ぎ
			swordTransform.rotate.y += attackTimer_ * 8.0f;
			swordTransform.rotate.x -= attackTimer_ * 2.0f;
			break;

		case 2: // 2段目: 縦斬り
			swordTransform.rotate.x -= attackTimer_ * 10.0f;
			swordTransform.rotate.z -= attackTimer_ * 3.0f;
			break;

		case 3: // 3段目: 回転斬り
			swordTransform.rotate.y += attackTimer_ * 12.0f;
			swordTransform.rotate.z += attackTimer_ * 4.0f;
			break;

		case 4: // 4段目: 強烈な叩きつけ
			swordTransform.rotate.x -= attackTimer_ * 15.0f;
			swordTransform.rotate.z -= attackTimer_ * 8.0f;
			swordTransform.translate.y += attackTimer_ * 2.0f; // 上から振り下ろす
			break;

		case 5:                                 // 落下攻撃: 真下に突き刺す
			swordTransform.rotate.x -= 3.14f;   // 真下に向ける
			swordTransform.translate.y -= 1.0f; // 下に伸ばす
			// 回転エフェクト
			swordTransform.rotate.y += attackTimer_ * 20.0f;
			break;

		case 6: // 重撃: 大振りの横薙ぎ
			swordTransform.rotate.y += attackTimer_ * 18.0f;
			swordTransform.rotate.z += attackTimer_ * 10.0f;
			swordTransform.translate.x += attackTimer_ * 3.0f; // 大きく振る
			break;

		default:
			swordTransform.rotate.x -= attackTimer_ * 3.0f;
			swordTransform.rotate.z -= attackTimer_ * 6.0f;
			break;
		}

		// 攻撃終了判定
		if (attackTimer_ >= attackDuration_) {
			isAttacking_ = false;
			attackTimer_ = 0.0f;
		}
	}

	swordObject_->SetTransform(swordTransform);
	swordObject_->SetCamera(camera);
	swordObject_->Update();

#ifdef _DEBUG
	debugBox_->SetTransform(swordTransform);
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