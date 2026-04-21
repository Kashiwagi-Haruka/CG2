#define NOMINMAX
#include "DebugCamera.h"

#include "Function.h"
#include "Input.h"
#include "WinApp.h"
#include <algorithm>
#include <cmath>

namespace {
Matrix4x4 MakeCameraViewMatrix(const Transform& transform) {
	const Matrix4x4 inverseTranslate = Function::MakeTranslateMatrix(-transform.translate.x, -transform.translate.y, -transform.translate.z);
	const Matrix4x4 inverseRotateY = Function::MakeRotateYMatrix(-transform.rotate.y);
	const Matrix4x4 inverseRotateX = Function::MakeRotateXMatrix(-transform.rotate.x);
	const Matrix4x4 inverseRotateZ = Function::MakeRotateZMatrix(-transform.rotate.z);
	return Function::Multiply(Function::Multiply(Function::Multiply(inverseTranslate, inverseRotateY), inverseRotateX), inverseRotateZ);
}

Vector3 GetRightAxis(const Matrix4x4& rotationMatrix) { return Function::Normalize({rotationMatrix.m[0][0], rotationMatrix.m[0][1], rotationMatrix.m[0][2]}); }

Vector3 GetForwardAxis(const Matrix4x4& rotationMatrix) { return Function::Normalize({rotationMatrix.m[2][0], rotationMatrix.m[2][1], rotationMatrix.m[2][2]}); }

void ZoomPivotOffsetDistance(Vector3& translation, float zoomDelta) {
	constexpr float kMinDistance = 0.5f;
	constexpr float kMaxDistance = 500.0f;
	if (zoomDelta == 0.0f) {
		return;
	}

	float currentDistance = std::sqrtf(Function::LengthSquared(translation));
	if (currentDistance <= 0.0001f) {
		translation = {0.0f, 0.0f, -1.0f};
		currentDistance = 1.0f;
	}

	const float nextDistance = std::clamp(currentDistance + zoomDelta, kMinDistance, kMaxDistance);
	translation = Function::Normalize(translation) * nextDistance;
}
} // namespace

void DebugCamera::Initialize() {
	// 既定値を実画面サイズに合わせて設定
	fovY_ = 0.45f * 3.14159265f;
	aspectRatio_ = float(WinApp::kClientWidth) / float(WinApp::kClientHeight);
	nearZ_ = 0.1f;
	farZ_ = 10000.0f;
	matRot_ = Function::MakeIdentity4x4();
	scale_ = {1.0f, 1.0f, 1.0f};
	translation_ = transform_.translate;
	Update();
}

void DebugCamera::SetTransform(const Transform& transform) {
	// 受け取った姿勢を基準に、Pivot操作用の内部状態を再初期化
	transform_ = transform;
	pivot_ = {0.0f, 0.0f, 0.0f};
	scale_ = transform.scale;
	translation_ = transform.translate;
	matRot_ = Function::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, transform.rotate, {0.0f, 0.0f, 0.0f});
}

void DebugCamera::SetRotation(const Vector3& rotation) {
	// 回転角を反映し、回転行列を同期
	transform_.rotate = rotation;
	matRot_ = Function::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, transform_.rotate, {0.0f, 0.0f, 0.0f});
}

void DebugCamera::Update() {
	// 左ドラッグ: 向いている方向基準の自由移動 / Ctrl+左ドラッグ: 距離ズーム / Shift+左ドラッグ: 平面移動 / ホイール: 距離ズーム
	const float moveSpeed = 0.02f;
	const float dragZoomSpeed = 0.08f;
	const float wheelZoomSpeed = 0.02f;

	Input* input = Input::GetInstance();
	const Vector2 mouseMove = input->GetMouseMove();
	const bool isLeftDrag = input->PushMouseButton(Input::MouseButton::kLeft);
	const bool isShift = input->PushKey(DIK_LSHIFT) || input->PushKey(DIK_RSHIFT);
	const bool isCtrl = input->PushKey(DIK_LCONTROL) || input->PushKey(DIK_RCONTROL);

	float zoomDelta = 0.0f;
	if (isLeftDrag && isShift) {
		const Vector3 right = GetRightAxis(matRot_);
		Vector3 forward = GetForwardAxis(matRot_);
		forward.y = 0.0f;
		if (Function::LengthSquared(forward) > 0.0f) {
			forward = Function::Normalize(forward);
		}
		translation_ += right * (mouseMove.x * moveSpeed);
		translation_ += forward * (-mouseMove.y * moveSpeed);
	} else if (isLeftDrag && isCtrl) {
		zoomDelta += mouseMove.y * dragZoomSpeed;
	} else if (isLeftDrag) {
		const Vector3 right = GetRightAxis(matRot_);
		const Vector3 forward = GetForwardAxis(matRot_);
		translation_ += right * (mouseMove.x * moveSpeed);
		translation_ += forward * (-mouseMove.y * moveSpeed);
	}

	const float wheelDelta = input->GetMouseWheelDelta();
	if (wheelDelta != 0.0f) {
		zoomDelta += -wheelDelta * wheelZoomSpeed;
	}

	ZoomPivotOffsetDistance(translation_, zoomDelta);

	transform_.scale = scale_;
	transform_.translate = translation_ + pivot_;
	worldMatrix_ = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = MakeCameraViewMatrix(transform_);
	projectionMatrix_ = Function::MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearZ_, farZ_);
	viewProjectionMatrix_ = Function::Multiply(viewMatrix_, projectionMatrix_);
}