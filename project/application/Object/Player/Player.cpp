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
	hp_ = parameters_.hpMax_;
	bulletVelocity_ = {0, 0, 0};
	isDash = false;
	isJump = false;
	isfalling = false;
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
					velocity_.x += -parameters_.accelationRate;
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
				velocity_.x += parameters_.accelationRate;
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
		}	
	}
	
}
void Player::Attack() {

	// --- 発射方向入力 ---
	Vector3 shotDir = {0, 0, 0};
	if (GameBase::GetInstance()->PushKey(DIK_A))
		shotDir.x = -1;
	if (GameBase::GetInstance()->PushKey(DIK_D))
		shotDir.x = 1;
	if (GameBase::GetInstance()->PushKey(DIK_W))
		shotDir.y = 1;

	// 正規化（0ベクトルの場合は右向き）
	if (Function::Length(shotDir) > 0.0f) {
		shotDir = Function::Normalize(shotDir);
	} else {
		shotDir = {1, 0, 0}; // デフォルト＝右
	}

	// --- チャージ開始 ---
	if (GameBase::GetInstance()->TriggerKey(DIK_J)) {
		if (bullet_)
			delete bullet_;
		bullet_ = new PlayerBullet();
		bullet_->Initialize(camera_);
	}

	// --- チャージ中（弾をプレイヤー前に配置） ---
	if (GameBase::GetInstance()->PushKey(DIK_J)) {
		if (bullet_) {
			bullet_->SetVelocity(shotDir * 0.5f); // 発射方向を渡す
			bullet_->Charge(transform_.translate, shotDir);
			state_ = State::kAttacking;
		}
	}

	// --- 発射 ---
	if (GameBase::GetInstance()->ReleaseKey(DIK_J)) {
		if (bullet_) {
			bullet_->Fire();
		}
		state_ = State::kIdle;
	}

	if (bullet_) {
		bullet_->Update(camera_);
	}
}

void Player::Update(){

	Attack();
	Move();
	Jump();
	Falling();
	if (parameters_.EXP >= parameters_.MaxEXP) {
		parameters_.Level++;
		parameters_.EXP -= parameters_.MaxEXP;
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
void Player::Draw() {
	
	GameBase::GetInstance()->ModelCommonSet();
	playerObject_->Draw();
	
	if (bullet_) {
	bullet_->Draw();
	}
}
Vector3 Player::GetBulletPosition() { return bullet_->GetPosition(); }