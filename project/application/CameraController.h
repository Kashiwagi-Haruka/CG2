#pragma once
#include "Transform.h"
class Camera;
class CameraController {

	Transform transform_{};
	Camera* camera_ = nullptr;


	public:
	~CameraController();
	void Initialize();
	void Update();
	void SetCamera(Camera* camera);
	Camera* GetCamera();
	Transform GetTransform() { return transform_;}
	void SetTransform(Transform transform) { transform_ = transform; }

};
