#pragma once
#include "Transform.h"
class Object3d;
class GameBase;
class Camera;
class Skydome {

	Transform transform_{};
	Object3d* skydomeObject_ = nullptr;
	Camera* camera_ = nullptr;

	public:
	~Skydome();
	void Initialize(GameBase* gameBase,Camera* camera);
	void Update();
	void Draw(GameBase* gameBase);


};
