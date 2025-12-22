#define NOMINMAX
#include "Player.h"
#include "Camera.h"
#include "Function.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include "Object/MapchipField.h"
#include <algorithm>
#include <numbers>
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

Player::Player() {
	ModelManeger::GetInstance()->LoadModel("playerModel");
	playerObject_ = std::make_unique<Object3d>();
	sword_ = std::make_unique<PlayerSword>();
}

Player::~Player() {}

void Player::Initialize(Camera* camera) {

	state_ = State::kIdle;
	velocity_ = {0.0f, 0.0f, 0.0f};
	transform_ = {
	    .scale{1.0f, 1.0f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{0.0f, 1.5f, 0.0f}
    };

	playerObject_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	playerObject_->SetModel("playerModel");

	camera_ = camera;
	playerObject_->SetCamera(camera_);
	sword_->Initialize();
	sword_->SetCamera(camera_);
	isAlive = true;
	parameters_ = SetInit();
	hp_ = parameters_.hpMax_;
	bulletVelocity_ = {0, 0, 0};
	isDash = false;
	isJump = false;
	isfalling = false;
	isLevelUP = false;
	rotateTimer = 0.1f;

	// コンボ関連の初期化
	comboStep_ = 0;
	comboTimer_ = 0.0f;
	comboWindow_ = 0.6f; // コンボ受付時間（秒）
	isAttacking_ = false;
	canCombo_ = false;
}

void Player::Move() {

	lastTapTimeA_++;
	lastTapTimeD_++;
	lastTapTimeS_++;
	lastTapTimeW_++;

	// 入力方向を記録する変数
	Vector3 inputDirection = {0.0f, 0.0f, 0.0f};
	bool hasInput = false;

	if (GameBase::GetInstance()->PushKey(DIK_A) || GameBase::GetInstance()->PushKey(DIK_D)) {

		if (!GameBase::GetInstance()->PushKey(DIK_D)) {
			if (GameBase::GetInstance()->TriggerKey(DIK_A)) {
				if (lastTapTimeA_ < parameters_.doubleTapThreshold_) {
					isDash = true;
				}

				lastTapTimeA_ = 0.0f;
			}
			if (GameBase::GetInstance()->PushKey(DIK_A)) {
				velocity_.x += -parameters_.accelationRate * (parameters_.SpeedUp + 1);
				inputDirection.x = -1.0f;
				hasInput = true;
			}
		}

		if (!GameBase::GetInstance()->PushKey(DIK_A)) {
			if (GameBase::GetInstance()->TriggerKey(DIK_D)) {
				if (lastTapTimeD_ < parameters_.doubleTapThreshold_) {
					isDash = true;
				}
				lastTapTimeD_ = 0.0f;
			}
			if (GameBase::GetInstance()->PushKey(DIK_D)) {
				velocity_.x += parameters_.accelationRate * (parameters_.SpeedUp + 1);
				inputDirection.x = 1.0f;
				hasInput = true;
			}
		}
	}
	if (GameBase::GetInstance()->PushKey(DIK_W) || GameBase::GetInstance()->PushKey(DIK_S)) {

		if (!GameBase::GetInstance()->PushKey(DIK_W)) {
			if (GameBase::GetInstance()->TriggerKey(DIK_S)) {
				if (lastTapTimeS_ < parameters_.doubleTapThreshold_) {
					isDash = true;
				}

				lastTapTimeS_ = 0.0f;
			}
			if (GameBase::GetInstance()->PushKey(DIK_S)) {
				velocity_.z += -parameters_.accelationRate * (parameters_.SpeedUp + 1);
				inputDirection.z = -1.0f;
				hasInput = true;
			}
		}

		if (!GameBase::GetInstance()->PushKey(DIK_S)) {
			if (GameBase::GetInstance()->TriggerKey(DIK_W)) {
				if (lastTapTimeW_ < parameters_.doubleTapThreshold_) {
					isDash = true;
				}
				lastTapTimeW_ = 0.0f;
			}
			if (GameBase::GetInstance()->PushKey(DIK_W)) {
				velocity_.z += parameters_.accelationRate * (parameters_.SpeedUp + 1);
				inputDirection.z = 1.0f;
				hasInput = true;
			}
		}
	}

	// 入力がある場合、その方向に向きを回転
	if (hasInput) {
		// 入力方向から目標角度を計算
		float targetAngle = std::atan2(inputDirection.x, inputDirection.z);

		// 現在の角度と目標角度の差を計算
		float angleDiff = targetAngle - transform_.rotate.y;

		// 角度を-π〜πの範囲に正規化
		while (angleDiff > std::numbers::pi_v<float>) {
			angleDiff -= 2.0f * std::numbers::pi_v<float>;
		}
		while (angleDiff < -std::numbers::pi_v<float>) {
			angleDiff += 2.0f * std::numbers::pi_v<float>;
		}

		// 滑らかに回転
		transform_.rotate.y = Function::Lerp(transform_.rotate.y, transform_.rotate.y + angleDiff, rotateTimer);
	}

	if (!GameBase::GetInstance()->PushKey(DIK_A) && !GameBase::GetInstance()->PushKey(DIK_D) && !GameBase::GetInstance()->PushKey(DIK_W) && !GameBase::GetInstance()->PushKey(DIK_S)) {
		isDash = false;
	}

	if (GameBase::GetInstance()->TriggerKey(DIK_SPACE)) {
		if (!isfalling && !isJump) {
			isJump = true;
		}
	}

	if (!GameBase::GetInstance()->PushKey(DIK_A) && !GameBase::GetInstance()->PushKey(DIK_D)) {
		velocity_.x *= (1.0f - parameters_.decelerationRate);
		if (velocity_.x > -0.01f && velocity_.x < 0.01f) {
			velocity_.x = 0.0f;
		}
	}
	if (!GameBase::GetInstance()->PushKey(DIK_W) && !GameBase::GetInstance()->PushKey(DIK_S)) {
		velocity_.z *= (1.0f - parameters_.decelerationRate);
		if (velocity_.z > -0.01f && velocity_.z < 0.01f) {
			velocity_.z = 0.0f;
		}
	}

	velocity_.x = std::clamp(velocity_.x, -parameters_.accelationMax, parameters_.accelationMax);
	velocity_.z = std::clamp(velocity_.z, -parameters_.accelationMax, parameters_.accelationMax);
	if (isDash) {
		velocity_.x *= parameters_.dashMagnification;
		velocity_.z *= parameters_.dashMagnification;
	}
	transform_.translate += velocity_;
	if (GameBase::GetInstance()->PushKey(DIK_A)) {
		bulletVelocity_.x = -1;
	}
	if (GameBase::GetInstance()->PushKey(DIK_D)) {
		bulletVelocity_.x = 1;
	}
}

void Player::Jump() {

	if (isJump) {
		velocity_.y = parameters_.jumpPower;

		if (jumpTimer >= parameters_.jumpTimerMax) {
			jumpTimer = 0.0f;
			isJump = false;
			isfalling = true;
		} else {
			jumpTimer += 0.1f * (1 / 60.0f);
		}
	}
}

void Player::Falling() {

	if (isfalling) {

		velocity_.y -= parameters_.gravity;

		if (transform_.translate.y <= 1.5f) {
			transform_.translate.y = 1.5f;
			velocity_.y = 0.0f;
			isfalling = false;

			usedAirAttack = false;
		}
	}
}

void Player::Attack() {

	// コンボタイマーの更新
	if (comboTimer_ > 0.0f) {
		comboTimer_ -= 1.0f / 60.0f;
		if (comboTimer_ <= 0.0f) {
			// タイムアウト: コンボリセット
			comboStep_ = 0;
			canCombo_ = false;
		}
	}

	// 攻撃入力の処理
	if (GameBase::GetInstance()->TriggerKey(DIK_J)) {

		// 攻撃中でない、またはコンボ可能な状態の場合
		if (!isAttacking_ || canCombo_) {

			// 次のコンボ段階に進む
			comboStep_++;
			if (comboStep_ > 4) {
				comboStep_ = 1; // 4段階目の後は最初に戻る
			}

			// 攻撃状態の設定
			isAttacking_ = true;
			canCombo_ = false;          // 連打防止: 一旦コンボ不可に
			comboTimer_ = comboWindow_; // コンボ受付時間をリセット

			// 攻撃ステートの設定
			switch (comboStep_) {
			case 1:
				attackState_ = AttackState::kWeakAttack1;
				sword_->StartAttack(1); // 1段階目
				break;
			case 2:
				attackState_ = AttackState::kWeakAttack2;
				sword_->StartAttack(2); // 2段階目
				break;
			case 3:
				attackState_ = AttackState::kWeakAttack3;
				sword_->StartAttack(3); // 3段階目
				break;
			case 4:
				attackState_ = AttackState::kWeakAttack4;
				sword_->StartAttack(4); // 4段階目（フィニッシュ）
				break;
			}
		}
	}

	// 攻撃モーションが終了したか確認
	if (isAttacking_ && !sword_->IsAttacking()) {
		isAttacking_ = false;
		canCombo_ = true; // 次のコンボ入力を受け付ける

		// 4段階目が終わったらコンボリセット
		if (comboStep_ >= 4) {
			comboStep_ = 0;
			canCombo_ = false;
			comboTimer_ = 0.0f;
		}
	}
}

void Player::Update() {

	Attack();
	Move();
	Jump();
	Falling();

	// --- レベルアップ処理 ---
	if (parameters_.Level < parameters_.MaxLevel) {

		if (parameters_.EXP >= parameters_.MaxEXP) {

			parameters_.EXP -= parameters_.MaxEXP;
			parameters_.Level++;

			isLevelUP = true;
		}
	} else {
		if (parameters_.EXP > parameters_.MaxEXP) {
			parameters_.EXP = parameters_.MaxEXP;
		}
	}

	playerObject_->SetCamera(camera_);

	playerObject_->SetTransform(transform_);
	playerObject_->Update();
	sword_->SetCamera(camera_);
	sword_->Update(transform_);

#ifdef USE_IMGUI

	if (ImGui::Begin("Player")) {

		ImGui::DragInt("HP", &hp_);
		ImGui::DragFloat3("plPos", &transform_.translate.x);
		ImGui::DragFloat3("plRot", &transform_.rotate.x);
		ImGui::DragFloat("rotateSpeed", &rotateTimer, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("comboWindow", &comboWindow_, 0.01f, 0.1f, 2.0f);
		ImGui::Text("J = FIRE , WASD = MOVE , SPACE = JUMP");
		ImGui::Text("isDash: %d", isDash);
		ImGui::Text("Combo Step: %d / 4", comboStep_);
		ImGui::Text("Combo Timer: %.2f", comboTimer_);
		ImGui::Text("Can Combo: %s", canCombo_ ? "YES" : "NO");
	}
	ImGui::End();

#endif //

	parameters_.hpMax_ = parameters_.hpMax_ * (1 + parameters_.HPUp);

	// 死
	if (hp_ <= 0) {
		isAlive = false;
	}

	//  --- ダメージ後の無敵時間 ---
	if (isInvincible_) {
		invincibleTimer_ -= 1.0f / 60.0f;
		if (invincibleTimer_ <= 0.0f) {
			isInvincible_ = false;
		}
	}
}

void Player::EXPMath() { parameters_.EXP += 50; }

void Player::Draw() {

	GameBase::GetInstance()->ModelCommonSet();
	playerObject_->Draw();
	sword_->Draw();
}