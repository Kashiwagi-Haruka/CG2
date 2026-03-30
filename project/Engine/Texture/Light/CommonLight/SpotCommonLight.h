#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <cstddef>
#include <cstdint>

constexpr size_t kMaxSpotLights = 20;
struct SpotCommonLight {
	Vector4 color;         // 色
	Vector3 position;      // 位置
	float intensity;       // 輝度
	Vector3 direction;     // スポットライトの方向
	float distance;        // ライトの輝く最大距離
	float decay;           // 減衰率
	float cosAngle;        // スポットライトの余弦
	float cosFalloffStart; // スポットライトの減衰開始角度の余弦
	int32_t shadowEnabled;
	float padding;
};

struct SpotCommonLightCount {
	uint32_t count;
	float padding[3];
};