#include "Skeleton.h"
#include "Function.h"
#include <cmath>

namespace {
constexpr float kJointRadius = 0.03f;
constexpr float kBoneThickness = 0.015f;
constexpr float kBoneLengthEpsilon = 0.0001f;
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

Vector3 GetJointWorldPosition(const Joint& joint, const Matrix4x4& objectMatrix) {
	Matrix4x4 worldMatrix = Function::Multiply(joint.skeletonSpaceMatrix, objectMatrix);
	return Function::TransformVM({0.0f, 0.0f, 0.0f}, worldMatrix);
}

void UpdateSkeletonAnimation(Skeleton& skeleton, const Animation::AnimationData& animation, float& animationTime, float deltaTime) {
	if (animation.duration <= 0.0f) {
		return;
	}

	animationTime += deltaTime;
	animationTime = std::fmod(animationTime, animation.duration);
	ApplyAnimation(skeleton, animation, animationTime);
	UpdateSkeleton(skeleton);
}

void DrawSkeletonBones(const Skeleton& skeleton, const Matrix4x4& objectMatrix, Primitive* jointPrimitive, Primitive* bonePrimitive, const Vector4& jointColor, const Vector4& boneColor) {
	if (!jointPrimitive || !bonePrimitive) {
		return;
	}

	jointPrimitive->SetColor(jointColor);
	bonePrimitive->SetColor(boneColor);
	jointPrimitive->SetEnableLighting(false);
	bonePrimitive->SetEnableLighting(false);

	for (const Joint& joint : skeleton.joints) {
		Vector3 jointPosition = GetJointWorldPosition(joint, objectMatrix);
		jointPrimitive->SetTransform({
		    .scale{kJointRadius,    kJointRadius,    kJointRadius   },
		    .rotate{0.0f,            0.0f,            0.0f           },
		    .translate{jointPosition.x, jointPosition.y, jointPosition.z},
		});
		jointPrimitive->Update();
		jointPrimitive->Draw();

		if (!joint.parent.has_value()) {
			continue;
		}

		const Joint& parentJoint = skeleton.joints[*joint.parent];
		Vector3 parentPosition = GetJointWorldPosition(parentJoint, objectMatrix);
		Vector3 direction = jointPosition - parentPosition;
		float length = Function::Length(direction);
		if (length <= kBoneLengthEpsilon) {
			continue;
		}

		Vector3 center = {
		    (jointPosition.x + parentPosition.x) * 0.5f,
		    (jointPosition.y + parentPosition.y) * 0.5f,
		    (jointPosition.z + parentPosition.z) * 0.5f,
		};
		Vector3 rotation = Function::DirectionToRotation(Function::Normalize(direction), {1.0f, 0.0f, 0.0f});
		bonePrimitive->SetTransform({
		    .scale{length,     kBoneThickness, kBoneThickness},
		    .rotate{rotation.x, rotation.y,     rotation.z    },
		    .translate{center.x,   center.y,       center.z      },
		});
		bonePrimitive->Update();
		bonePrimitive->Draw();
	}
}