#pragma once
#include "Vector3.h"
#include "Transform.h"
class GameBase;
class Camera;
class Object3d;

class PlayerBullet {

	Transform transform_;

	Camera* camera_ = nullptr;
	Object3d* object_ = nullptr;
	Vector3 direction_;


	public:

	~PlayerBullet();
	void Initialize(GameBase* gameBase,Camera* camera, Vector3 emitPos, Vector3 direction);
	void Update(GameBase* gameBase);
	void Draw(GameBase* gameBase);

};
