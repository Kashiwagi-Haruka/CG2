#define NOMINMAX
#include "Player.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include <algorithm>
#include "Camera.h"
#include "Object/MapchipField.h"
#include <numbers>
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI


Player::Player() {
	ModelManeger::GetInstance()->LoadModel("playerModel");
	playerObject_ = std::make_unique<Object3d>();
	sword_ = std::make_unique<PlayerSword>();
}

Player::~Player(){

	
	
	
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
}
void Player::Move(){


		lastTapTimeA_ ++;
		lastTapTimeD_ ++;
	    lastTapTimeS_++;
	    lastTapTimeW_++;

		

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
			    }
		    }

		    if (!GameBase::GetInstance()->PushKey(DIK_S)) {
			    // D ダブルタップ
			    if (GameBase::GetInstance()->TriggerKey(DIK_W)) {
				    if (lastTapTimeW_ < parameters_.doubleTapThreshold_) {
					    isDash = true;
				    }
				    lastTapTimeW_ = 0.0f;
			    }
			    if (GameBase::GetInstance()->PushKey(DIK_W)) {
				    velocity_.z += parameters_.accelationRate * (parameters_.SpeedUp + 1);
			    }
		    }
	    }
		if (!GameBase::GetInstance()->PushKey(DIK_A) && !GameBase::GetInstance()->PushKey(DIK_D)&&!GameBase::GetInstance()->PushKey(DIK_W)&&!GameBase::GetInstance()->PushKey(DIK_S)) {
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
	// E キーでスキル攻撃へ
	if (GameBase::GetInstance()->TriggerKey(DIK_E)) {
		attackState_ = Player::AttackState::kSkillAttack;
	}
	// Q キーで必殺技へ
	if (GameBase::GetInstance()->TriggerKey(DIK_Q)) {
		attackState_ = Player::AttackState::kSpecialAttack;
	}
	switch (attackState_) {
	case Player::AttackState::kNone:
		// J キーで弱攻撃1へ
		if (GameBase::GetInstance()->TriggerKey(DIK_J)){
			attackState_ = Player::AttackState::kWeakAttack1;
		}
		

		break;
	case Player::AttackState::kWeakAttack1:
		break;
	case Player::AttackState::kWeakAttack2:
		break;
	case Player::AttackState::kWeakAttack3:
		break;
	case Player::AttackState::kWeakAttack4:
		break;
	case Player::AttackState::kStrongAttack:
		break;
	case Player::AttackState::kSkillAttack:
		break;
	case Player::AttackState::kSpecialAttack:
		break;
	default:
		break;
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
	sword_->SetCamera(camera_);
	sword_->Update(transform_);
	
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
	sword_->Draw();

}
