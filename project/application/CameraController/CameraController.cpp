#include "CameraController.h"
#include "Camera.h"
#include "GameBase.h"
#include <imgui.h>
CameraController::~CameraController() {}
void CameraController::Initialize() {

	transform_ = {
	    .scale{1.0f,        1.0f,      1.0f  },
        .rotate{orbitPitch_, orbitYaw_, 0.0f  },
        .translate{0.0f,        10.0f,     -50.0f}
    };

	camera_ = std::make_unique<Camera>();
	camera_->SetTransform(transform_);
}
void CameraController::Update() {

#ifdef USE_IMGUI

	/*if (ImGui::Begin("CameraController")) {
		ImGui::DragFloat3("CameraScale", &transform_.scale.x, 0.01f);
		ImGui::DragFloat("OrbitYaw", &orbitYaw_, 0.01f);
		ImGui::DragFloat("OrbitPitch", &orbitPitch_, 0.01f);
		ImGui::DragFloat3("CameraTranslate", &transform_.translate.x, 0.1f);
	}
	ImGui::End();*/

#endif
	const Vector2 mouseMove = GameBase::GetInstance()->GetMouseMove();
	orbitYaw_ += mouseMove.x * mouseSensitivity_;
	orbitPitch_ += mouseMove.y * mouseSensitivity_;

	const float maxPitch = 1.2f;
	const float minPitch = -1.2f;
	if (orbitPitch_ > maxPitch) {
		orbitPitch_ = maxPitch;
	}
	if (orbitPitch_ < minPitch) {
		orbitPitch_ = minPitch;
	}

	const float distance = 20.0f;
	Vector3 orbitDir = {sinf(orbitYaw_) * cosf(orbitPitch_), -sinf(orbitPitch_), cosf(orbitYaw_) * cosf(orbitPitch_)};

	transform_.translate = playerPos - orbitDir * distance;
	transform_.rotate = {orbitPitch_, orbitYaw_, 0.0f};

	camera_->SetTransform(transform_);
	camera_->Update();
}
void CameraController::SpecialAttackUpdate() {}
Camera* CameraController::GetCamera() { return camera_.get(); }