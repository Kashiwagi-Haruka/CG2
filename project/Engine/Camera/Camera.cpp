#include "Camera.h"
#include "Function.h"

namespace {
// カメラTransformからビュー行列を作成する補助関数
Matrix4x4 MakeCameraViewMatrix(const Transform& transform) {
	const Matrix4x4 inverseTranslate = Function::MakeTranslateMatrix(-transform.translate.x, -transform.translate.y, -transform.translate.z);
	const Matrix4x4 inverseRotateY = Function::MakeRotateYMatrix(-transform.rotate.y);
	const Matrix4x4 inverseRotateX = Function::MakeRotateXMatrix(-transform.rotate.x);
	const Matrix4x4 inverseRotateZ = Function::MakeRotateZMatrix(-transform.rotate.z);
	return Function::Multiply(Function::Multiply(Function::Multiply(inverseTranslate, inverseRotateY), inverseRotateX), inverseRotateZ);
}
} // namespace

Camera::Camera()
    : transform_({
          {1.0f, 1.0f, 1.0f},
          {0.0f, 0.0f, 0.0f},
          {0.0f, 0.0f, 0.0f}
}),
      fovY(0.45f * 3.14159265f), aspectRatio(float(WinApp::kClientWidth) / float(WinApp::kClientHeight)), nearZ(0.1f), farZ(10000.0f),
      worldMatrix_(Function::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, transform_.rotate, transform_.translate)), viewMatrix_(MakeCameraViewMatrix(transform_)),
      projectionMatrix_(Function::MakePerspectiveFovMatrix(fovY, aspectRatio, nearZ, farZ)), viewProjectionMatrix_(Function::Multiply(viewMatrix_, projectionMatrix_)) {}

void Camera::Update() {
	// 現在のTransformと投影設定をもとに各行列を更新
	worldMatrix_ = Function::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, transform_.rotate, transform_.translate);
	viewMatrix_ = MakeCameraViewMatrix(transform_);
	projectionMatrix_ = Function::MakePerspectiveFovMatrix(fovY, aspectRatio, nearZ, farZ);
	viewProjectionMatrix_ = Function::Multiply(viewMatrix_, projectionMatrix_);
}

void Camera::SetViewProjectionMatrix(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {
	// 受け取った行列を採用し、派生行列と位置情報を同期
	viewMatrix_ = viewMatrix;
	projectionMatrix_ = projectionMatrix;
	viewProjectionMatrix_ = Function::Multiply(viewMatrix_, projectionMatrix_);
	worldMatrix_ = Function::Inverse(viewMatrix_);
	transform_.translate = {worldMatrix_.m[3][0], worldMatrix_.m[3][1], worldMatrix_.m[3][2]};
}