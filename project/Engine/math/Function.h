#pragma once
#include "Matrix4x4.h"
#include "Vector3.h"
#include <cstdint>


namespace Function {
	
	float Dot(const Vector3& v1, const Vector3& v2);

	Vector3 Normalize(const Vector3& v);

	Matrix4x4 MakeRotateXMatrix(float radian);
	Matrix4x4 MakeRotateYMatrix(float radian);
	Matrix4x4 MakeRotateZMatrix(float radian);
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
	Vector3 Cross(const Vector3& v1, const Vector3& v2);
    Vector3 Distance(const Vector3& pos1, const Vector3& pos2);

	Vector3 TransformVM(const Vector3& vector, const Matrix4x4& matrix4x4);
	Matrix4x4 Inverse(const Matrix4x4& m);
	Matrix4x4 MakeTranslateMatrix(Vector3 translate);
    Matrix4x4 MakeTranslateMatrix(float x, float y, float z); 
	Matrix4x4 MakeScaleMatrix(Vector3 scale);
	Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector3 rotate, Vector3 translate);

	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
	Matrix4x4 Transpose(const Matrix4x4& m);
    Matrix4x4 MakeIdentity4x4();
    
	}

Vector3 operator+(const Vector3& v1, const Vector3& v2);
Vector3 operator-(const Vector3& v1, const Vector3& v2);
Vector3 operator*(const Vector3& v, float scalar);
Vector3 operator+=(Vector3& v1, const Vector3& v2);
inline float LengthSquared(const Vector3& v) { return v.x * v.x + v.y * v.y + v.z * v.z; }

