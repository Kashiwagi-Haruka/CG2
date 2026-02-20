#pragma once
#include "Matrix4x4.h"
#include "Vector3.h"
#include "Vector4.h"
#include <cstdint>
#include <numbers>

namespace Function {

inline constexpr float kPi = std::numbers::pi_v<float>;

float Dot(const Vector3& v1, const Vector3& v2);
float Length(const Vector3& v);

// direction（向きたい方向）から回転角を計算する
// forwardAxis：モデルの前方向（Cube は X 軸→ {1,0,0}）
Vector3 DirectionToRotation(const Vector3& direction, const Vector3& forwardAxis);
Vector3 Normalize(const Vector3& v);

Matrix4x4 MakeRotateXMatrix(float radian);
Matrix4x4 MakeRotateYMatrix(float radian);
Matrix4x4 MakeRotateZMatrix(float radian);
Matrix4x4 MakeRotateMatrix(const Vector4& rotate);
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
Vector3 Cross(const Vector3& v1, const Vector3& v2);
Vector3 Distance(const Vector3& pos1, const Vector3& pos2);

Vector3 TransformVM(const Vector3& vector, const Matrix4x4& matrix4x4);
Matrix4x4 Inverse(const Matrix4x4& m);
Matrix4x4 MakeTranslateMatrix(Vector3 translate);
Matrix4x4 MakeTranslateMatrix(float x, float y, float z);
Matrix4x4 MakeScaleMatrix(Vector3 scale);
Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector3 rotate, Vector3 translate);
Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector4 rotate, Vector3 translate);

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
Matrix4x4 Transpose(const Matrix4x4& m);
Vector3 Lerp(const Vector3& start, const Vector3& end, float ratio);
float Lerp(float start, float end, float ratio);
Matrix4x4 MakeIdentity4x4();

} // namespace Function
Vector3 operator+(const Vector3& v1, const Vector3& v2);
Vector3 operator-(const Vector3& v1, const Vector3& v2);
Vector3 operator*(const Vector3& v, float scalar);
Vector3 operator+=(Vector3& v1, const Vector3& v2);
inline float LengthSquared(const Vector3& v) { return v.x * v.x + v.y * v.y + v.z * v.z; }
