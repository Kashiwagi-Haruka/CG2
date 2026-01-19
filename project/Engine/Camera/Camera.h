#pragma once
#include "Matrix4x4.h"
#include "Transform.h"
#include "WinApp.h"

class Camera {

	Transform transform_;
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 viewProjectionMatrix_;

	float fovY = 0.45f;                   // 視野角（縦方向）
	float aspectRatio = 1280.0f / 720.0f; // アスペクト比
	float nearZ = 0.1f;                   // ニアクリップ距離
	float farZ = 1000.0f;                 // ファークリップ距離

public:
	Camera();
	void Update();
	void SetViewProjectionMatrix(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix);

	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
	void SetTransform(const Transform& transform) { transform_ = transform; }
	void SetFovY(float fovY) { this->fovY = fovY; }
	void SetAspectRatio(float aspectRatio) { this->aspectRatio = aspectRatio; }
	void SetNearClip(float nearZ) { this->nearZ = nearZ; }
	void SetFarClip(float farZ) { this->farZ = farZ; }

	const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }

	const Vector3& GetRotate() const { return transform_.rotate; }
	const Vector3& GetTranslate() const { return transform_.translate; }
	Vector3 GetWorldTranslate() const { return {worldMatrix_.m[3][0], worldMatrix_.m[3][1], worldMatrix_.m[3][2]}; }

	const Transform& GetTransform() const { return transform_; }
};