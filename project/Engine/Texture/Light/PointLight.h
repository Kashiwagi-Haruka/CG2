#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <cstddef>
#include <cstdint>
#include <vector>

class PointLight {
	struct Data {
		Vector4 color;
		Vector3 position;
		float intensity;
		float radius;
		float decay;
		float padding[2];
	};
	std::vector<Data> pointLights_;
};
