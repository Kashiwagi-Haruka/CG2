#pragma once
#include "Vector3.h"
#include "Transform.h"
#include "Object3d.h"
#include "Input.h"
class GameBase;
class Camera;
class PlayerBullet;
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
	float accelationMax = 0.5f;
	float decelerationRate = 0.15f;

	float jumpPower = 0.7f;
	float jumpDuration = 0.5f;
	float jumpTimer = 0.0f;
	float jumpTimerMax = 0.02f;
	float gravity = 0.98f/10.0f;


	Vector3 velocity_;

	Transform transform_;

	Object3d* object_;
	Camera* camera_;
	PlayerBullet* bullet_;

	public:

	~Player();
	void Initialize(GameBase* gameBase,Camera* camera);
	void Move(GameBase* gameBase);
	void Attack(GameBase* gameBase);
	void Update(GameBase* gameBase);
	void Draw(GameBase* gameBase);

};
