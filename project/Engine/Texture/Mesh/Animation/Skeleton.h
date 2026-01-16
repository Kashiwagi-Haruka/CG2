#pragma once
#include "Animation/Animation.h"
#include "Matrix4x4.h"
#include "Model/Model.h"
#include "QuaternionTransform.h"
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

struct Joint {
	QuaternionTransform transform{};
	Matrix4x4 localMatrix{};
	Matrix4x4 skeletonSpaceMatrix{};
	std::string name;
	std::vector<int32_t> children;
	int32_t index = -1;
	std::optional<int32_t> parent;
};

struct Skeleton {
	int32_t root = -1;
	std::map<std::string, int32_t> jointMap;
	std::vector<Joint> joints;
};

Skeleton CreateSkeleton(const Model::Node& rootNode);
void UpdateSkeleton(Skeleton& skeleton);
void ApplyAnimation(Skeleton& skeleton, const Animation::AnimationData& animation, float animationTime);