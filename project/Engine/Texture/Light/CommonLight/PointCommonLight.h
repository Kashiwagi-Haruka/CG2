#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <cstddef>
#include <cstdint>
constexpr size_t kMaxPointLights = 20;
struct PointCommonLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	float radius;
	float decay;
	int32_t shadowEnabled;
	float padding;
};
struct PointCommonLightCount {
	uint32_t count;
	float padding[3];
};
struct PointCommonLightSet {
	PointCommonLight lights[kMaxPointLights];
	int count;
	float padding[3];
};