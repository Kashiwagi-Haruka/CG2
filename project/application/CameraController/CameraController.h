#pragma once
#include "Transform.h"
#include <memory>
class Camera;
class CameraController {

	Transform transform_{};
	std::unique_ptr<Camera> camera_;
	float mouseSensitivity_ = 0.002f;
	Vector3 playerPos = {0.0f, 0.0f, 0.0f};
	float orbitYaw_ = 0.0f;
	float orbitPitch_ = 0.15f;

public:
	~CameraController();
	void Initialize();
	void Update();
	void SpecialAttackUpdate();

	Camera* GetCamera();
	Transform GetTransform() { return transform_; }
	void SetTransform(Transform transform) { transform_ = transform; }
	void SetPlayerPos(const Vector3& pos) { playerPos = pos; }
	void SetTranslate(const Vector3& translate) {
		transform_.translate.x = translate.x;
		transform_.translate.z = translate.z;
	}
};
