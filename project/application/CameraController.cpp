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

	if (ImGui::Begin("CameraController")) {
		ImGui::DragFloat3("CameraScale", &transform_.scale.x, 0.01f);
		ImGui::DragFloat("OrbitYaw", &orbitYaw_, 0.01f);
		ImGui::DragFloat("OrbitPitch", &orbitPitch_, 0.01f);
		ImGui::DragFloat3("CameraTranslate", &transform_.translate.x, 0.1f);
		
			ImGui::Text("worldMatrix[0][0]=%f,worldMatrix[0][1]=%f,worldMatrix[0][2]=%f,worldMatrix[0][3]=%f", 
				camera_->GetWorldMatrix().m[0][0], camera_->GetWorldMatrix().m[0][1],
			    camera_->GetWorldMatrix().m[0][2], camera_->GetWorldMatrix().m[0][3]);
			ImGui::Text(
			    "worldMatrix[1][0]=%f,worldMatrix[1][1]=%f,worldMatrix[1][2]=%f,worldMatrix[1][3]=%f", 
				camera_->GetWorldMatrix().m[1][0], camera_->GetWorldMatrix().m[1][1],
			    camera_->GetWorldMatrix().m[1][2], camera_->GetWorldMatrix().m[1][3]);
			ImGui::Text(
			    "worldMatrix[2][0]=%f,worldMatrix[2][1]=%f,worldMatrix[2][2]=%f,worldMatrix[2][3]=%f", 
				camera_->GetWorldMatrix().m[2][0], camera_->GetWorldMatrix().m[2][1],
			    camera_->GetWorldMatrix().m[2][2], camera_->GetWorldMatrix().m[2][3]);
			ImGui::Text(
			    "worldMatrix[3][0]=%f,worldMatrix[3][1]=%f,worldMatrix[3][2]=%f,worldMatrix[3][3]=%f", 
				camera_->GetWorldMatrix().m[3][0], camera_->GetWorldMatrix().m[3][1],
			    camera_->GetWorldMatrix().m[3][2], camera_->GetWorldMatrix().m[3][3]);
			
					ImGui::Text(
		        "viewMatrix[0][0]=%f,viewMatrix[0][1]=%f,viewMatrix[0][2]=%f,viewMatrix[0][3]=%f", camera_->GetViewMatrix().m[0][0], camera_->GetViewMatrix().m[0][1], camera_->GetViewMatrix().m[0][2],
		        camera_->GetViewMatrix().m[0][3]);
		    ImGui::Text(
		                "viewMatrix[1][0]=%f,viewMatrix[1][1]=%f,viewMatrix[1][2]=%f,viewMatrix[1][3]=%f", camera_->GetViewMatrix().m[1][0], camera_->GetViewMatrix().m[1][1],
		                camera_->GetViewMatrix().m[1][2], camera_->GetViewMatrix().m[1][3]);
		    ImGui::Text(
		        "viewMatrix[2][0]=%f,viewMatrix[2][1]=%f,viewMatrix[2][2]=%f,viewMatrix[2][3]=%f", camera_->GetViewMatrix().m[2][0], camera_->GetViewMatrix().m[2][1], camera_->GetViewMatrix().m[2][2],
		        camera_->GetViewMatrix().m[2][3]);
		    ImGui::Text(
		        "viewMatrix[3][0]=%f,viewMatrix[3][1]=%f,viewMatrix[3][2]=%f,viewMatrix[3][3]=%f", camera_->GetViewMatrix().m[3][0], camera_->GetViewMatrix().m[3][1], camera_->GetViewMatrix().m[3][2],
		        camera_->GetViewMatrix().m[3][3]);
		
	}
	ImGui::End();

#endif
	const Vector2 mouseMove = GameBase::GetInstance()->GetMouseMove();
	orbitYaw_ += mouseMove.x * mouseSensitivity_;
	orbitPitch_ += mouseMove.y * mouseSensitivity_;
	//if (GameBase::GetInstance()->PushKey(DIK_LEFT)) {
	//	orbitYaw_ -= cameraSpeed_;
	//}
	//if (GameBase::GetInstance()->PushKey(DIK_RIGHT)) {
	//	orbitYaw_ += cameraSpeed_;
	//}
	//if (GameBase::GetInstance()->PushKey(DIK_UP)) {
	//	orbitPitch_ += cameraSpeed_;
	//}
	//if (GameBase::GetInstance()->PushKey(DIK_DOWN)) {
	//	orbitPitch_ -= cameraSpeed_;
	//}
	//if (GameBase::GetInstance()->PushKey(DIK_LEFT)) {
	//	transform_.rotate.y -= cameraSpeed_;
	//}
	//if (GameBase::GetInstance()->PushKey(DIK_RIGHT)) {
	//	transform_.rotate.y += cameraSpeed_;
	//}
	//if (GameBase::GetInstance()->PushKey(DIK_UP)) {
	//	transform_.rotate.x += cameraSpeed_;
	//}
	//if (GameBase::GetInstance()->PushKey(DIK_DOWN)) {
	//	transform_.rotate.x -= cameraSpeed_;
	//}
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