#pragma once
#include "Function.h"
class DebugCamera {

	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 viewProjectionMatrix_;

public:
	void Initialize();
	void Update();
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }

	// DebugCamera.h
	Matrix4x4 GetViewMatrix() const { return viewMatrix_; }
	Matrix4x4 GetProjectionMatrix() const { return projectionMatrix_; }
	void SetPivot(const Vector3& pivot) { pivot_ = pivot; }
	void SetTranslation(const Vector3& translation) { translation_ = translation; }
	Vector3 pivot_ = {0, 0, 0};
	Matrix4x4 matRot_ = {};
	Vector3 rotation_ = {0, 0, 0};
	Vector3 scale_ = {1, 1, 1};
	Vector3 translation_ = {50, 50, -30};
};