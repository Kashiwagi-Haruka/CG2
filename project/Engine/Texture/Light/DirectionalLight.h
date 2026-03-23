#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <cstdint>
#include <d3d12.h>
#include <wrl.h>

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
	int32_t shadowEnabled;
	float padding[3];
};