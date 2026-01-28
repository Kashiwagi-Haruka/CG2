#include "Camera.h"
#include "Function.h"

Camera::Camera()
    : transform_({
          {1.0f, 1.0f, 1.0f},
          {0.0f, 0.0f, 0.0f},
          {0.0f, 0.0f, 0.0f}
}),
      fovY(0.45f), aspectRatio(float(WinApp::kClientWidth) / float(WinApp::kClientHeight)), nearZ(0.1f), farZ(10000.0f),
      worldMatrix_(Function::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, transform_.rotate, transform_.translate)), viewMatrix_(Function::Inverse(worldMatrix_)),
      projectionMatrix_(Function::MakePerspectiveFovMatrix(fovY, aspectRatio, nearZ, farZ)), viewProjectionMatrix_(Function::Multiply(viewMatrix_, projectionMatrix_)) {}

void Camera::Update() {

	worldMatrix_ = Function::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, transform_.rotate, transform_.translate);
	viewMatrix_ = Function::Inverse(worldMatrix_);
	projectionMatrix_ = Function::MakePerspectiveFovMatrix(fovY, aspectRatio, nearZ, farZ);
	viewProjectionMatrix_ = Function::Multiply(viewMatrix_, projectionMatrix_);
}

void Camera::SetViewProjectionMatrix(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {
	viewMatrix_ = viewMatrix;
	projectionMatrix_ = projectionMatrix;
	viewProjectionMatrix_ = Function::Multiply(viewMatrix_, projectionMatrix_);
	worldMatrix_ = Function::Inverse(viewMatrix_);
	transform_.translate = {worldMatrix_.m[3][0], worldMatrix_.m[3][1], worldMatrix_.m[3][2]};
}