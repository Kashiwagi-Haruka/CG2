#pragma once
#include "Matrix4x4.h"
#include "Vector3.h"

class Function {

	Vector3 scale_;
	Vector3 rotate_;
	Vector3 translate_;
	Matrix4x4 worldMatrix_;

	static const int kRowHeight=20;
	static const int kColumnWidth=60;

	public:

	Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);
	Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
	Matrix4x4 Inverse(const Matrix4x4& m);
	Matrix4x4 Transpose(const Matrix4x4& m);
	Matrix4x4 MakeIdentity();

	//平行移動
	Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
	//拡大縮小
	Matrix4x4 MakeScaleMatrix(const Vector3& scale);
	//座標変換
	Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);
	//X軸回転
	Matrix4x4 MakeRotateXMatrix(float radian);
	//Y軸回転
	Matrix4x4 MakeRotateYMatrix(float radian);
	// Z軸回転
	Matrix4x4 MakeRotateZMatrix(float radian);

	//affine行列
	Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector3 rotate, Vector3 translate);

	


	Function();
	~Function();
	void Math();





};
