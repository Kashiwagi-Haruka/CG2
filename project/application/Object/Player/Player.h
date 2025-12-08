#pragma once
#include "Vector3.h"
#include "Transform.h"
#include "Object3d.h"
#include "Input.h"
#include "PlayerParameters.h"
class GameBase;
class Camera;
class PlayerBullet;
class MapchipField;
class BulletManager;

class Player {
	
	enum class State {
		kIdle,
		kRunning,
		kJumping,
		kFalling, 
		kAttacking

	};
	State state_;

	Parameters parameters_;

	struct Select{
		
	};

	float jumpTimer = 0.0f;
	int hp_; // プレイヤーHP

	bool isAlive;
	bool isInvincible_ = false;
	float invincibleTimer_ = 0.0f;
	
	// ダブルタップ判定用
	float lastTapTimeA_ = 0.0f;
	float lastTapTimeD_ = 0.0f;
	
	bool isDash = false;
	bool isJump = false;
	bool isfalling = false;

	Vector3 velocity_;
	Vector3 bulletVelocity_;

	Transform transform_;

	Object3d* playerObject_ = nullptr;

	BulletManager* bulletManager_ = nullptr;

	Camera* camera_;
	
	MapchipField* map_ = nullptr;
	bool isAirAttack = false;
	int airAttackIndex = 0; // 今何発目か（0〜3）
	float airAttackTimer = 0.0f;
	bool isSelect_;
	bool isLevelUP;
	bool usedAirAttack = false;

	public:

	Player();
	~Player();
	void Initialize(Camera* camera);
	void Move();
	void Attack();
	void Update();
	void Draw();
	void Jump();
	void Falling();
	void SetBulletManager(BulletManager* manager) { bulletManager_ = manager; }

	void SetCamera(Camera* camera) { camera_ = camera;}
	void SetMap(MapchipField* map) { map_ = map; }
	Vector3 GetPosition() { return transform_.translate; }
	Vector3 GetVelocity() { return velocity_; }
	Vector3 GetBulletPosition();
	bool GetIsAlive() { return isAlive; }
	Parameters GetParameters() { return parameters_; }
	void SetParameters(const Parameters& p) { parameters_ = p; }

	void Damage(int amount) {
		if (!isInvincible_) {
			hp_ -= amount;
			isInvincible_ = true;
			invincibleTimer_ = 1.0f; // 1秒無敵
		}
	}
	bool GetSelect() { return isSelect_; };
	int GetHP() const { return hp_; }
	int GetHPMax() const { return parameters_.hpMax_; }
	void IsLevelUp(bool lv) { isLevelUP = lv; }
	bool GetLv() { return isLevelUP; }
	void EXPMath();
};
