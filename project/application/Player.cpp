#define NOMINMAX
#include "Player.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include <algorithm>
#include "PlayerBullet.h"
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
		.translate{0.0f, 0.0f, 0.0f}
	};
	ModelManeger::GetInstance()->LoadModel("playerModel");
	ModelManeger::GetInstance()->LoadModel("skyDome");
	
	playerObject_ = new Object3d();
	
	
	playerObject_->Initialize(gameBase->GetObject3dCommon());
	
	
	playerObject_->SetModel("playerModel");
	
	
	camera_ = camera;
	playerObject_->SetCamera(camera_);
	
	bullet_ = new PlayerBullet();
	
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
		if (transform_.translate.y <= 0.0f) {
			transform_.translate.y = 0.0f;
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

}
void Player::Attack(GameBase* gameBase){
	
	if (gameBase->TriggerKey(DIK_J)) {
			Vector3 direction = {1.0f, 0.0f, 0.0f};
			
			bullet_->Initialize(gameBase, camera_, transform_.translate, direction);
	}
	if (gameBase->PushKey(DIK_J)) {
	
		if (bullet_) {
			bullet_->Charge(transform_.translate);
		}
	}
	if (gameBase->ReleaseKey(DIK_J)) {
		if (bullet_) {
			bullet_->Fire();
		}
	}
		
	
}
void Player::Update(GameBase* gameBase){

	Move(gameBase);
	
	Attack(gameBase);
	playerObject_->SetCamera(camera_);
	
	playerObject_->SetScale(transform_.scale);
	playerObject_->SetRotate(transform_.rotate);
	playerObject_->SetTranslate(transform_.translate);
	playerObject_->Update();

	
	
	//if (bullet_) {
	//	bullet_->Update(gameBase);
	//}

}
void Player::Draw(GameBase* gameBase) {
	
	gameBase->ModelCommonSet();
	playerObject_->Draw();
	
	//if (bullet_) {
	//bullet_->Draw(gameBase);
	//}
}