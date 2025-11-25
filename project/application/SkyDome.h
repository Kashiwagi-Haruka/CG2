#pragma once
#include "Input.h"
#include "Object3d.h"
#include "Transform.h"
class GameBase;
class Camera;
class SkyDome {

	private:

	Transform transform_;
	Object3d* skyDomeObject_ = nullptr;

	Camera* camera_;
	public:
	SkyDome();
	~SkyDome();
	void Initialize(Camera* camera);
	void Update();
	void Draw();
	void SetCamera(Camera* camera) { camera_ = camera; }

};
