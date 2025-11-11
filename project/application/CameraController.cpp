#include "CameraController.h"
#include "Camera.h"

CameraController::~CameraController() {
	delete camera_; 
}
void CameraController::Initialize() {

	camera_ = new Camera();
	camera_->SetTranslate({0, 0, -50});
}
void CameraController::Update() {



	camera_->Update();

}
void CameraController::SetCamera(Camera* camera) {
	camera_ = camera; }
Camera* CameraController::GetCamera() {
	return camera_;}