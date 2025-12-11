#pragma once
#include "Transform.h"
#include <memory>
class Camera;
class CameraController {

	Transform transform_{};
	std::unique_ptr<Camera> camera_;


	public:
	~CameraController();
	void Initialize();
	void Update();
	
	Camera* GetCamera();
	Transform GetTransform() { return transform_;}
	void SetTransform(Transform transform) { transform_ = transform; }
	void SetTranslate(const Vector3& translate) { 
		transform_.translate.x = translate.x;
		transform_.translate.y = translate.y;
	}
};
