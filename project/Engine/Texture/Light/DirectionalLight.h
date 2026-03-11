#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <d3d12.h>
#include <wrl.h>

class DirectionalLight {
	Vector4 color;		//光の色(RGBA)
	Vector3 direction;	// 光の向き
	float intensity;	// 光の強さ

	public:

	DirectionalLight(); // デフォルトコンストラクタ
	Vector4 GetColor() const { return color; } // 色を取得する関数
	Vector3 GetDirection() const { return direction; } // 方向を取得する関数
	float GetIntensity() const { return intensity; }   // 強さを取得する関数

	void SetColor(const Vector4& newColor) { color = newColor; } // 色を設定する関数
	void SetDirection(const Vector3& newDirection) { direction = newDirection; } // 方向を設定する関数
	void SetIntensity(float newIntensity) { intensity = newIntensity; }          // 強さを設定する関数

};
