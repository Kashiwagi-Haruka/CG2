#pragma once
#include "Transform.h"
#include <memory>
class Camera;
class CameraController {

	Transform transform_{};
	std::unique_ptr<Camera> camera_;
	float cameraSpeed_ = 0.1f;
	Vector3 playerPos = {0.0f, 0.0f, 0.0f};
	float playerYaw = 0.0f;

	public:
	~CameraController();
	void Initialize();
	void Update();
	void SpecialAttackUpdate();
	
	Camera* GetCamera();
	Transform GetTransform() { return transform_;}
	void SetTransform(Transform transform) { transform_ = transform; }
	void SetPlayerPos(const Vector3& pos) { playerPos = pos; }
	void SetPlayerYaw(float yaw) { playerYaw = yaw; }
	void SetTranslate(const Vector3& translate) { 
		transform_.translate.x = translate.x;
		transform_.translate.z = translate.z;
	}
};

