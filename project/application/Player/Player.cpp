#define NOMINMAX
#include "Player.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include <algorithm>
#include "PlayerBullet.h"
#include "Camera.h"
#include "MapchipField.h"
#include <numbers>
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI


Player::Player(){
	ModelManeger::GetInstance()->LoadModel("playerModel");
	

	playerObject_ = new Object3d();
	
}
Player::~Player(){
	if (bullet_) {
	delete bullet_;
	}
	delete playerObject_;
	
	
}
void Player::Initialize(GameBase* gameBase,Camera* camera){

	state_ = State::kIdle;
	velocity_ = { 0.0f, 0.0f, 0.0f };
	transform_ = {
		.scale{1.0f, 1.0f, 1.0f},
		.rotate{0.0f, 0.0f, 0.0f},
		.translate{0.0f, 2.0f, 0.0f}
	};
	
	
	
	
	playerObject_->Initialize(gameBase->GetObject3dCommon());
	
	
	playerObject_->SetModel("playerModel");
	
	
	camera_ = camera;
	playerObject_->SetCamera(camera_);
	
	isAlive = true;
	hp_ = hpMax_;
	bulletVelocity_ = {0, 0, 0};
	
}
void Player::Move(GameBase* gameBase){
	switch (state_) {
	case Player::State::kIdle:
		
		if (gameBase->PushKey(DIK_A) || gameBase->PushKey(DIK_D)) {
			state_ = State::kRunning;
		}
		if (gameBase->TriggerKey(DIK_SPACE)) {
			state_ = State::kJumping;
		}
		break;
	case Player::State::kRunning:
		if (gameBase->PushKey(DIK_A)) {
			velocity_.x += -accelationRate;
		}
		if (gameBase->PushKey(DIK_D)) {
			velocity_.x += accelationRate;
		}

		if (gameBase->TriggerKey(DIK_SPACE)) {
			state_ = State::kJumping;
		}

		break;
	case Player::State::kJumping:

		velocity_.y = jumpPower;

		if (jumpTimer >= jumpTimerMax) {
			jumpTimer = 0.0f;
			state_ = State::kFalling;
		} else {
			jumpTimer += 0.1f * (1 / 60.0f);
		}

		break;
	case Player::State::kFalling:
		velocity_.y -= gravity;
		if (transform_.translate.y <= 1.5f) {
			transform_.translate.y = 1.5f;
			velocity_.y = 0.0f;
			state_ = State::kIdle;
		}
		break;
	case Player::State::kAttacking:
	default:
		break;
	}
	if (!gameBase->PushKey(DIK_A) && !gameBase->PushKey(DIK_D)) {
		velocity_.x *= (1.0f - decelerationRate);
		if (velocity_.x > -0.01f && velocity_.x < 0.01f) {
			velocity_.x = 0.0f;
			if (state_==State::kRunning) {
			state_ = State::kIdle;
			}
		}
	}
	velocity_.x = std::clamp(velocity_.x, -accelationMax, accelationMax);
	transform_.translate += velocity_;
	if (gameBase->PushKey(DIK_A)) {
		bulletVelocity_.x = -1;
	}
	if (gameBase->PushKey(DIK_D)) {
		bulletVelocity_.x = 1;
	}
	if (gameBase->PushKey(DIK_W)) {
		bulletVelocity_.y = 1;
	}
	
	
}
void Player::Attack(GameBase* gameBase) {

	// --- 発射方向入力 ---
	Vector3 shotDir = {0, 0, 0};
	if (gameBase->PushKey(DIK_A))
		shotDir.x = -1;
	if (gameBase->PushKey(DIK_D))
		shotDir.x = 1;
	if (gameBase->PushKey(DIK_W))
		shotDir.y = 1;

	// 正規化（0ベクトルの場合は右向き）
	if (Function::Length(shotDir) > 0.0f) {
		shotDir = Function::Normalize(shotDir);
	} else {
		shotDir = {1, 0, 0}; // デフォルト＝右
	}

	// --- チャージ開始 ---
	if (gameBase->TriggerKey(DIK_J)) {
		if (bullet_)
			delete bullet_;
		bullet_ = new PlayerBullet();
		bullet_->Initialize(gameBase, camera_);
	}

	// --- チャージ中（弾をプレイヤー前に配置） ---
	if (gameBase->PushKey(DIK_J)) {
		if (bullet_) {
			bullet_->SetVelocity(shotDir * 0.5f); // 発射方向を渡す
			bullet_->Charge(transform_.translate, shotDir);
			state_ = State::kAttacking;
		}
	}

	// --- 発射 ---
	if (gameBase->ReleaseKey(DIK_J)) {
		if (bullet_) {
			bullet_->Fire();
		}
		state_ = State::kIdle;
	}

	if (bullet_) {
		bullet_->Update(camera_);
	}
}

void Player::Update(GameBase* gameBase){

	Attack(gameBase);
	Move(gameBase);
	
	
	playerObject_->SetCamera(camera_);
	
	playerObject_->SetScale(transform_.scale);
	playerObject_->SetRotate(transform_.rotate);
	playerObject_->SetTranslate(transform_.translate);
	playerObject_->Update();

	
	#ifdef USE_IMGUI

	if (ImGui::Begin("Player")) {
		
	ImGui::DragInt("HP", &hp_);
		ImGui::Text("J = FIRE , WASD = MOVE , SPACE = JUMP");
	
	}
	ImGui::End();

	#endif // 

	
	
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
void Player::Draw(GameBase* gameBase) {
	
	gameBase->ModelCommonSet();
	playerObject_->Draw();
	
	if (bullet_) {
	bullet_->Draw(gameBase);
	}
}
Vector3 Player::GetBulletPosition() { return bullet_->GetPosition(); }