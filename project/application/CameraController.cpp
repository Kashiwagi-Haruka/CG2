#include "CameraController.h"
#include "Camera.h"
#include <imgui.h>
CameraController::~CameraController() {
	delete camera_; 
}
void CameraController::Initialize() {

	transform_ = {
		.scale{1.0f, 1.0f, 1.0f},
		.rotate{0.0f, 0.0f, 0.0f},
		.translate{0.0f, 5.0f, -50.0f}
    };

	camera_ = new Camera();
	camera_->SetTransform(transform_);
}
void CameraController::Update() {

	#ifdef USE_IMGUI

	if (ImGui::Begin("CameraController")) {
		ImGui::Text("Transform");
		ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
		ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &transform_.translate.x, 0.1f);
	}
	ImGui::End();
	

	#endif
	camera_->SetTransform(transform_);
	camera_->Update();

}
void CameraController::SetCamera(Camera* camera) {
	camera_ = camera; }
Camera* CameraController::GetCamera() {
	return camera_;}