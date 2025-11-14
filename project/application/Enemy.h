#pragma once
#include "Transform.h"
#include "Vector3.h"
class GameBase;
class Camera;
class Object3d;

class Enemy {

	Object3d* object_ = nullptr;
	Camera* camera_ = nullptr;

public:

	~Enemy();
	void Initialize(GameBase* gameBase,Camera* camera);
	void Update(GameBase* gameBase);
	void Draw(GameBase* gameBase);
	void SetCamera(Camera* camera) { camera_ = camera;}

};
