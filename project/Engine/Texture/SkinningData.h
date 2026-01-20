#pragma once
#include "Matrix4x4.h"
#include <cstdint>
#include <vector>
struct VertexWeightData {
	float weight;
	uint32_t vertexIndex;
};
struct JointWeightData {
	Matrix4x4 inverseBindPoseMatrix;
	std::vector<VertexWeightData> vertexWeights;
};