#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <cstddef>
constexpr size_t kMaxPointLights = 4;
struct PointLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	float radius;
	float decay;
	float padding[2];
};

struct PointLightSet {
	PointLight lights[kMaxPointLights];
	int count;
	float padding[3];
};