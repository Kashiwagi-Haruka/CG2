#pragma once
#include "Vector3.h"
#include "Transform.h"
class GameBase;
class Camera;
class Object3d;

class PlayerBullet {

	Transform bulletTransform_;
	Transform lineTransform_;

	Camera* camera_ = nullptr;
	Object3d* bulletObject_ = nullptr;
	Object3d* lineObject_ = nullptr;
	Vector3 direction_;
	Vector3 velocity_;

	public:

	~PlayerBullet();
	void Initialize(GameBase* gameBase,Camera* camera, Vector3 emitPos, Vector3 direction);
	void Update(GameBase* gameBase);
	void Charge(Vector3 playerPos);
	void Fire();
	void Draw(GameBase* gameBase);
	void SetVelocity(Vector3 velocity);
};
