#define NOMINMAX
#include "Player.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include <algorithm>
#include "PlayerBullet.h"
#include "Camera.h"
#include "Object/MapchipField.h"
#include <numbers>
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI
#include "BulletManager.h"

Player::Player(){
	ModelManeger::GetInstance()->LoadModel("playerModel");
	

	playerObject_ = new Object3d();
	
	
}
Player::~Player(){

	delete playerObject_;
	
	
}
void Player::Initialize(Camera* camera){

	state_ = State::kIdle;
	velocity_ = { 0.0f, 0.0f, 0.0f };
	transform_ = {
		.scale{1.0f, 1.0f, 1.0f},
		.rotate{0.0f, 0.0f, 0.0f},
		.translate{0.0f, 1.5f, 0.0f}
	};
	
	
	
	
	playerObject_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	
	
	playerObject_->SetModel("playerModel");
	
	
	camera_ = camera;
	playerObject_->SetCamera(camera_);
	
	isAlive = true;
	parameters_ = SetInit();
	hp_ = parameters_.hpMax_;
	bulletVelocity_ = {0, 0, 0};
	isDash = false;
	isJump = false;
	isfalling = false;
	isLevelUP = false;
}
void Player::Move(){


		lastTapTimeA_ ++;
		lastTapTimeD_ ++;
		
		if (GameBase::GetInstance()->PushKey(DIK_A) || GameBase::GetInstance()->PushKey(DIK_D)) {
		
			if (!GameBase::GetInstance()->PushKey(DIK_D)) {
				if (GameBase::GetInstance()->TriggerKey(DIK_A)) {
					if (lastTapTimeA_ < parameters_.doubleTapThreshold_) {
						isDash = true;
					}
			
					lastTapTimeA_ = 0.0f;
				}
				if (GameBase::GetInstance()->PushKey(DIK_A)) {
					velocity_.x += -parameters_.accelationRate*(parameters_.SpeedUp+1);
				}
			}
			
			if (!GameBase::GetInstance()->PushKey(DIK_A)) {
				// D ダブルタップ
				if (GameBase::GetInstance()->TriggerKey(DIK_D)) {
					if (lastTapTimeD_ < parameters_.doubleTapThreshold_) {
						isDash = true;
					}
					lastTapTimeD_ = 0.0f;
				}
				if (GameBase::GetInstance()->PushKey(DIK_D)) {
				velocity_.x += parameters_.accelationRate*(parameters_.SpeedUp+1);
				}
			}
		}

		if (!GameBase::GetInstance()->PushKey(DIK_A) && !GameBase::GetInstance()->PushKey(DIK_D)) {
			isDash = false;
		}


		if (GameBase::GetInstance()->TriggerKey(DIK_SPACE)) {
			if (!isfalling&&!isJump) {
				isJump = true;
			}
		}

	if (!GameBase::GetInstance()->PushKey(DIK_A) && !GameBase::GetInstance()->PushKey(DIK_D)) {
		velocity_.x *= (1.0f - parameters_.decelerationRate);
		if (velocity_.x > -0.01f && velocity_.x < 0.01f) {
			velocity_.x = 0.0f;
	
		}
	}
	
	velocity_.x = std::clamp(velocity_.x, -parameters_.accelationMax, parameters_.accelationMax);
	if (isDash) {
		velocity_.x *= parameters_.dashMagnification;
	}
	transform_.translate += velocity_;
	if (GameBase::GetInstance()->PushKey(DIK_A)) {
		bulletVelocity_.x = -1;
	}
	if (GameBase::GetInstance()->PushKey(DIK_D)) {
		bulletVelocity_.x = 1;
	}
	if (GameBase::GetInstance()->PushKey(DIK_W)) {
		bulletVelocity_.y = 1;
	}
	
	
}
void Player::Jump(){

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
void Player::Falling(){

	if (isfalling) {
		
		velocity_.y -= parameters_.gravity;
		

		if (transform_.translate.y <= 1.5f) {
			transform_.translate.y = 1.5f;
			velocity_.y = 0.0f;
			isfalling = false;

			// ★ 地面に着いたので空中攻撃回数をリセット
			usedAirAttack = false;
		}
	}
	
}
void Player::Attack() {

	if (!bulletManager_)
		return;
	// --- 空中攻撃開始 ---
	// 落下中 かつ まだ空中攻撃を使っていない時だけ発動
	if (isfalling && !usedAirAttack && GameBase::GetInstance()->TriggerKey(DIK_J)) {

		isAirAttack = true;
		usedAirAttack = true; // ★ 空中攻撃を使ったことにする

		airAttackIndex = 0;
		airAttackTimer = 0.0f;

		isfalling = false; // 空中攻撃中は落下停止
		velocity_.y = 0.0f;
	}



	// --- 空中攻撃中 ---
	// --- 空中攻撃中 ---
	if (isAirAttack) {

		airAttackTimer += 1.0f / 60.0f; // 毎フレームタイマー進む

		// ★ 0.1秒ごとに1発撃つ
		if (airAttackTimer >= 0.2f) {
			// ★ AllowUp の値で本数可変
			int maxAirShots = parameters_.AllowUp + 1;
			airAttackTimer = 0.0f;

			// 縦に並べる位置
			Vector3 pos = transform_.translate;
			pos.y -= (float)airAttackIndex * 1.0f;

			// 発射方向：右下を基準に少し散らす
			float offset = (airAttackIndex - (maxAirShots - 1) * 0.5f) * 0.15f;
			Vector3 dir = {1, -1 + offset, 0};
			dir = Function::Normalize(dir);

			bulletManager_->Fire(pos, dir);


			airAttackIndex++;

			

			if (airAttackIndex >= maxAirShots) {
				isAirAttack = false;
				isfalling = true; // ← 落下再開
			}
		}

		return;
	}


	// --- 通常の地上攻撃（今のあなたの処理） ---
	Vector3 shotDir = {0, 0, 0};
	if (GameBase::GetInstance()->PushKey(DIK_A))
		shotDir.x = -1;
	if (GameBase::GetInstance()->PushKey(DIK_D))
		shotDir.x = 1;
	if (GameBase::GetInstance()->PushKey(DIK_W))
		shotDir.y = 1;

	if (Function::Length(shotDir) > 0.0f)
		shotDir = Function::Normalize(shotDir);
	else
		shotDir = {1, 0, 0};

	bulletManager_->SetPlayerPos(transform_.translate);
	bulletManager_->SetShotDir(shotDir);

	if (GameBase::GetInstance()->ReleaseKey(DIK_J)) {

		int arrowCount = parameters_.AllowUp+1; // 1 + AllowUp

		for (int i = 0; i < arrowCount; i++) {

			// 本数に応じて少し角度を散らす
			float offset = (i - (arrowCount - 1) * 0.5f) * 0.15f; // 角度差

			Vector3 dir2 = {shotDir.x, shotDir.y + offset, shotDir.z};
			dir2 = Function::Normalize(dir2);

			bulletManager_->Fire(transform_.translate, dir2);
		}
	}
}



void Player::Update(){
	
	Attack();
	Move();
	Jump();
	Falling();
	
// --- レベルアップ処理 ---
	if (parameters_.Level < parameters_.MaxLevel) {

		// ★ まだレベル上限に達していない場合だけレベルアップ判定する
		if (parameters_.EXP >= parameters_.MaxEXP) {

			parameters_.EXP -= parameters_.MaxEXP;
			parameters_.Level++;

			isLevelUP = true; // レベルアップ画面を出す
		}
	} else {
		// ★ MAX レベルのときはEXPが入っても処理しない
		if (parameters_.EXP > parameters_.MaxEXP) {
			parameters_.EXP = parameters_.MaxEXP;
		}
	}

	playerObject_->SetCamera(camera_);
	
	playerObject_->SetScale(transform_.scale);
	playerObject_->SetRotate(transform_.rotate);
	playerObject_->SetTranslate(transform_.translate);
	playerObject_->Update();

	
	#ifdef USE_IMGUI

	if (ImGui::Begin("Player")) {
		
	ImGui::DragInt("HP", &hp_);
		ImGui::DragFloat3("plPos", &transform_.translate.x);
		ImGui::Text("J = FIRE , WASD = MOVE , SPACE = JUMP");
		ImGui::Text("%d", isDash);
	}
	ImGui::End();

	#endif // 

	parameters_.hpMax_=parameters_.hpMax_*(1+parameters_.HPUp);

	
	//死
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

void Player::EXPMath(){ parameters_.EXP += 50; }
void Player::Draw() {
	
	GameBase::GetInstance()->ModelCommonSet();
	playerObject_->Draw();
	

}
