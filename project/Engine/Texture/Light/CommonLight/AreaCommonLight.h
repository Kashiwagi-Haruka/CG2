#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <cstddef>
#include <cstdint>
constexpr size_t kMaxAreaLights = 20;
struct AreaCommonLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	Vector3 normal;
	float width;
	float height;
	float radius;
	float decay;
	int32_t shadowEnabled;
	float padding[3];
};
struct AreaCommonLightCount {
	uint32_t count;
	float padding[3];
};