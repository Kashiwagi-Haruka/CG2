#include "Skeleton.h"
#include "Function.h"

namespace {
int32_t CreateJoint(const Model::Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints) {
	Joint joint;
	joint.name = node.name;
	joint.transform = node.transform;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = Function::MakeIdentity4x4();
	joint.index = static_cast<int32_t>(joints.size());
	joint.parent = parent;
	joints.push_back(joint);

	const int32_t jointIndex = joint.index;
	for (const Model::Node& child : node.children) {
		int32_t childIndex = CreateJoint(child, jointIndex, joints);
		joints[jointIndex].children.push_back(childIndex);
	}

	return jointIndex;
}
} // namespace

Skeleton CreateSkeleton(const Model::Node& rootNode) {
	Skeleton skeleton;
	skeleton.root = CreateJoint(rootNode, std::nullopt, skeleton.joints);

	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}

	return skeleton;
}

void UpdateSkeleton(Skeleton& skeleton) {
	for (Joint& joint : skeleton.joints) {
		joint.localMatrix = Function::MakeAffineMatrix(joint.transform.scale, joint.transform.quaternion, joint.transform.translate);
		if (joint.parent) {
			joint.skeletonSpaceMatrix = Function::Multiply(joint.localMatrix, skeleton.joints[*joint.parent].skeletonSpaceMatrix);
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void ApplyAnimation(Skeleton& skeleton, const Animation::AnimationData& animation, float animationTime) {
	for (Joint& joint : skeleton.joints) {
		auto it = animation.nodeAnimations.find(joint.name);
		if (it == animation.nodeAnimations.end()) {
			continue;
		}

		const Animation::NodeAnimation& nodeAnimation = it->second;
		if (!nodeAnimation.translate.keyframes.empty()) {
			joint.transform.translate = Animation::CalculateValue(nodeAnimation.translate, animationTime);
		}
		if (!nodeAnimation.rotation.keyframes.empty()) {
			joint.transform.quaternion = Animation::CalculateValue(nodeAnimation.rotation, animationTime);
		}
		if (!nodeAnimation.scale.keyframes.empty()) {
			joint.transform.scale = Animation::CalculateValue(nodeAnimation.scale, animationTime);
		}
	}
}