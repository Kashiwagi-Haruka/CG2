#pragma once
#include "Vector3.h"
#include "Transform.h"
#include "Object3d.h"
#include "Input.h"
class GameBase;
class Camera;
class PlayerBullet;
class MapchipField;
class Player {
	
	enum class State {
		kIdle,
		kRunning,
		kJumping,
		kFalling, 
		kAttacking

	};
	State state_;

	float accelationRate = 0.1f;
	float accelationMax = 0.25f;
	float decelerationRate = 0.15f;

	float jumpPower = 0.7f;
	float jumpDuration = 0.5f;
	float jumpTimer = 0.0f;
	float jumpTimerMax = 0.01f;
	float gravity = 0.98f/10.0f;
	float bulletRadius = 0.01f;

	int hp_; // プレイヤーHP
	int hpMax_ = 10000;
	bool isAlive;
	bool isInvincible_ = false;
	float invincibleTimer_ = 0.0f;
	
	// ダブルタップ判定用
	float lastTapTimeA_ = 0.0f;
	float lastTapTimeD_ = 0.0f;
	float dashMagnification = 2.0f;//ダッシュ倍率
	bool isDash = false;
	float doubleTapThreshold_ = 30.0f; //ダッシュの連打間隔


	Vector3 velocity_;
	Vector3 bulletVelocity_;

	Transform transform_;

	Object3d* playerObject_ = nullptr;

	
	Camera* camera_;
	PlayerBullet* bullet_;
	MapchipField* map_ = nullptr;

	public:

	Player();
	~Player();
	void Initialize(Camera* camera);
	void Move();
	void Attack();
	void Update();
	void Draw();
	void SetCamera(Camera* camera) { camera_ = camera;}
	void SetMap(MapchipField* map) { map_ = map; }
	Vector3 GetPosition() { return transform_.translate; }
	Vector3 GetVelocity() { return velocity_; }
	Vector3 GetBulletPosition();
	bool GetIsAlive() { return isAlive; }
	bool GetIsPlayerBullet() {
		if (bullet_) {
			return true;
		} else {
			return false;
		}
	}
	void Damage(int amount) {
		if (!isInvincible_) {
			hp_ -= amount;
			isInvincible_ = true;
			invincibleTimer_ = 1.0f; // 1秒無敵
		}
	}
	int GetHP() const { return hp_; }
	int GetHPMax() const { return hpMax_; }

	};
