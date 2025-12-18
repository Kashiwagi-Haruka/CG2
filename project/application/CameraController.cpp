#include "CameraController.h"
#include "Camera.h"
#include <imgui.h>
#include "GameBase.h"
CameraController::~CameraController() {
	
}
void CameraController::Initialize() {

	transform_ = {
		.scale{1.0f, 1.0f, 1.0f},
		.rotate{0.15f, 0.0f, 0.0f},
		.translate{0.0f, 10.0f, -50.0f}
    };

	camera_ = std::make_unique<Camera>();
	camera_->SetTransform(transform_);
}
void CameraController::Update() {

	#ifdef USE_IMGUI

	if (ImGui::Begin("CameraController")) {
		ImGui::DragFloat3("CameraScale", &transform_.scale.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("CameraTranslate", &transform_.translate.x, 0.1f);
	}
	ImGui::End();
	
	if (GameBase::GetInstance()->PushKey(DIK_LEFT)) {
		transform_.rotate.y -= cameraSpeed_;
	}
	if (GameBase::GetInstance()->PushKey(DIK_RIGHT)) {
		transform_.rotate.y += cameraSpeed_;
	}
	if (GameBase::GetInstance()->PushKey(DIK_UP)) {
		transform_.rotate.x -= cameraSpeed_;
	}
	if (GameBase::GetInstance()->PushKey(DIK_DOWN)) {
		transform_.rotate.x += cameraSpeed_;
	}
#endif
	const float distance = 20.0f; 
	const float height = 5.0f;   

	Vector3 backDir = {sinf(playerYaw), 0.0f, cosf(playerYaw)};

	transform_.translate = playerPos - backDir * distance + Vector3{0.0f, height, 0.0f};

	

	camera_->SetTransform(transform_);
	camera_->Update();

}

Camera* CameraController::GetCamera() {
	return camera_.get();}