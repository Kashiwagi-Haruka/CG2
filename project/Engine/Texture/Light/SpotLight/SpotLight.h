#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
class SpotLight {
	struct Light {
		Vector4 color;         // 色
		Vector3 position;      // 位置
		float intensity;       // 輝度
		Vector3 direction;     // スポットライトの方向
		float distance;        // ライトの輝く最大距離
		float decay;           // 減衰率
		float cosAngle;        // スポットライトの余弦
		float cosFalloffStart; // スポットライトの減衰開始角度の余弦
		float padding[2];
	};
	struct Data {
		Light light;
		std::string name; // ライトの名前（識別用）
	};

	std::vector<Data> spotLights_;

public:
	SpotLight(); // デフォルトコンストラクタ

	int GetSpotLightCount() const { return static_cast<int>(spotLights_.size()); } // スポットライトの数を取得する関数
	void AddSpotLight(std::string name);                                           // スポットライトを追加する関数
	void RemoveSpotLight(std::string name);                                            // スポットライトを削除する関数

	void SetSpotLightColor(const std::string& name, const Vector4& color);         // スポットライトの色を設定する関数
	void SetSpotLightPosition(const std::string& name, const Vector3& position);       // スポットライトの位置を設定する関数
	void SetSpotLightIntensity(const std::string& name, float intensity);          // スポットライトの輝度を設定する関数
	void SetSpotLightDirection(const std::string& name, const Vector3& direction);     // スポットライトの方向を設定する関数
	void SetSpotLightDistance(const std::string& name, float distance);            // スポットライトの最大距離を設定する関数
	void SetSpotLightDecay(const std::string& name, float decay);                      // スポットライトの減衰率を設定する関数
	void SetSpotLightAngle(const std::string& name, float angle);                  // スポットライトの角度を設定する関数
	void SetSpotLightFalloffStartAngle(const std::string& name, float falloffStartAngle); // スポットライトの減衰開始角度を設定する関数
	void SetSpotLightName(const std::string& oldName, const std::string& newName);        // スポットライトの名前を変更する関数
	void SetSpotLightProperties(
	    const std::string& name, const Vector4& color, const Vector3& position, float intensity, const Vector3& direction, float distance, float decay, float angle,
	    float falloffStartAngle); // スポットライトのすべてのプロパティを一度に設定する関数
	
	Vector4 GetSpotLightColor(const std::string& name) const; // スポットライトの色を取得する関数
	Vector3 GetSpotLightPosition(const std::string& name) const; // スポットライトの位置を取得する関数
	float GetSpotLightIntensity(const std::string& name) const;  // スポットライトの輝度を取得する関数
	Vector3 GetSpotLightDirection(const std::string& name) const; // スポットライトの方向を取得する関数
	float GetSpotLightDistance(const std::string& name) const;    // スポットライトの最大距離を取得する関数
	float GetSpotLightDecay(const std::string& name) const;       // スポットライトの減衰率を取得する関数
	float GetSpotLightAngle(const std::string& name) const;       // スポットライトの角度を取得する関数
	float GetSpotLightFalloffStartAngle(const std::string& name) const; // スポットライトの減衰開始角度を取得する関数

	void ClearSpotLights();                                                               // すべてのスポットライトをクリアする関数
};
