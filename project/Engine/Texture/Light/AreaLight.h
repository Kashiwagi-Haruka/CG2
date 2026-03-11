#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <cstddef>
#include <cstdint>
#include <vector>

class AreaLight {
	struct Data {
		Vector4 color;
		Vector3 position;
		float intensity;
		Vector3 normal;
		float width;
		float height;
		float radius;
		float decay;
		float padding;
	};
	
	std::vector<Data> areaLights_;

};
