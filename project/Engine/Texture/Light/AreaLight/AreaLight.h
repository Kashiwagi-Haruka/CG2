#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

class AreaLight {
	struct Light {
		Vector4 color;    // 色
		Vector3 position; // 位置
		float intensity;  // 輝度
		Vector3 normal;   // 面の法線ベクトル
		float width;      // 面の幅
		float height;     // 面の高さ
		float radius;     // ライトの輝く最大距離
		float decay;      // 減衰率
		float padding;    // パディング（16バイトアラインメントのため）
	};
	struct Data {
		Light light;
		std::string name; // ライトの名前（識別用）
	};
	
	std::vector<Data> areaLights_;

	public:
	
	AreaLight(); // デフォルトコンストラクタ

	int GetAreaLightCount() const { return static_cast<int>(areaLights_.size()); } // エリアライトの数を取得する関数
	void AddAreaLight(std::string name);                                           // エリアライトを追加する関数
	void RemoveAreaLight(std::string name);                                        // エリアライトを削除する関数

	void SetAreaLightColor(const std::string& name, const Vector4& color); // エリアライトの色を設定する関数
	void SetAreaLightPosition(const std::string& name, const Vector3& position); // エリアライトの位置を設定する関数
	void SetAreaLightIntensity(const std::string& name, float intensity);        // エリアライトの輝度を設定する関数
	void SetAreaLightNormal(const std::string& name, const Vector3& normal);     // エリアライトの法線を設定する関数
	void SetAreaLightSize(const std::string& name, float width, float height);   // エリアライトのサイズを設定する関数
	void SetAreaLightDistance(const std::string& name, float distance);          // エリアライトの最大距離を設定する関数
	void SetAreaLightDecay(const std::string& name, float decay);                // エリアライトの減衰率を設定する関数

	Vector4 GetAreaLightColor(const std::string& name) const; // エリアライトの色を取得する関数
	Vector3 GetAreaLightPosition(const std::string& name) const; // エリアライトの位置を取得する関数
	float GetAreaLightIntensity(const std::string& name) const;  // エリアライトの輝度を取得する関数
	Vector3 GetAreaLightNormal(const std::string& name) const;   // エリアライトの法線を取得する関数
	Vector2 GetAreaLightSize(const std::string& name) const;     // エリアライトのサイズを取得する関数
	float GetAreaLightDistance(const std::string& name) const;   // エリアライトの最大距離を取得する関数
	float GetAreaLightDecay(const std::string& name) const;      // エリアライトの減衰率を取得する関数

	void ClearAreaLights(); // すべてのエリアライトをクリアする関数
};
