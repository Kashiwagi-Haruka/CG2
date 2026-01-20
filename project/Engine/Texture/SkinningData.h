#pragma once
#include <cstdint>
#include <vector>
#include "Matrix4x4.h"
struct VertexWeightData{
	float weight;
	uint32_t vertexIndex;
};
struct JointWeightData {
	Matrix4x4 inverseBindPoseMatrix;
	std::vector<VertexWeightData> vertexWeights;
};