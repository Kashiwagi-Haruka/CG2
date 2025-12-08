#pragma once
#include <vector>
#include "Vector3.h"
#include "Transform.h"
class GameBase;
class Camera;
class Object3d;
class Enemy;
class PlayerBullet {

	int isCharge_;
	Transform transform_;

	Object3d* object_ = nullptr;
	Camera* camera_ = nullptr;
	Vector3 velocity_;

	bool isAir;
	bool isAirBullet =false;

	public:
	PlayerBullet();
	~PlayerBullet();
	void Initialize(Camera* camera);
	void Update(Camera* camera);
	void Charge(Vector3 playerPos, Vector3 direction);
	void Fire();
	
	void Draw();
	void SetVelocity(Vector3 velocity);
	Vector3 GetPosition() const { return transform_.translate; }
};
