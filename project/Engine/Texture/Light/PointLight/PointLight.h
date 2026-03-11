#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

class PointLight {
	struct Light {
		Vector4 color;    // 色
		Vector3 position; // 位置
		float intensity;  // 輝度
		float radius;     // ライトの輝く最大距離
		float decay;      // 減衰率
		float padding[2]; 
	};
	struct Data {
		Light light;
		std::string name; // ライトの名前（識別用）
	};
	std::vector<Data> pointLights_;

public:
	PointLight(); // デフォルトコンストラクタ

	int GetLightCount() const { return static_cast<int>(pointLights_.size()); } // ライトの数を取得する関数

	void AddPointLight(std::string name); // ライトを追加する関数
	void RemovePointLight(const std::string& name); // ライトを削除する関数

	void SetLightColor(const std::string& name, const Vector4& color); // ライトの色を設定する関数
	void SetLightPosition(const std::string& name, const Vector3& position); // ライトの位置を設定する関数
	void SetLightIntensity(const std::string& name, float intensity);        // ライトの輝度を設定する関数
	void SetLightRadius(const std::string& name, float radius);              // ライトの輝く最大距離を設定する関数
	void SetLightDecay(const std::string& name, float decay);                // ライトの減衰率を設定する関数
	void SetLightProperties(const std::string& name, const Vector4& color, const Vector3& position, float intensity, float radius, float decay); // ライトのプロパティを一括で設定する関数

	Vector4 GetLightColor(const std::string& name) const; // ライトの色を取得する関数
	Vector3 GetLightPosition(const std::string& name) const; // ライトの位置を取得する関数
	float GetLightIntensity(const std::string& name) const;  // ライトの輝度を取得する関数
	float GetLightRadius(const std::string& name) const;     // ライトの輝く最大距離を取得する関数
	float GetLightDecay(const std::string& name) const;      // ライトの減衰率を取得する関数

	void ClearLights(); // すべてのライトをクリアする関数

};
