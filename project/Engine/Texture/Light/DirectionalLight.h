#pragma once
#include "Vector4.h"
#include "Vector3.h"
#include <wrl.h>
#include <d3d12.h>

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

