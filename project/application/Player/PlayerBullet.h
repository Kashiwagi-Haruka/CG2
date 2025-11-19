#pragma once
#include "Vector3.h"
#include "Transform.h"
class GameBase;
class Camera;
class Object3d;

class PlayerBullet {

	int isCharge_;


	Transform bulletTransform_;
	Transform lineTransform_;

	Camera* camera_ = nullptr;
	Object3d* bulletObject_ = nullptr;
	Object3d* lineObject_ = nullptr;
	Vector3 velocity_;

	public:
	PlayerBullet();
	~PlayerBullet();
	void Initialize(GameBase* gameBase,Camera* camera);
	void Update(Camera* camera);
	void Charge(Vector3 playerPos, Vector3 direction);
	void Fire();
	void Draw(GameBase* gameBase);
	void SetVelocity(Vector3 velocity);
	Vector3 GetPosition() { return bulletTransform_.translate; }
};
