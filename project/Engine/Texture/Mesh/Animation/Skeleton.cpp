#include "Skeleton.h"
#include "Function.h"
#include "Object3d/Object3dCommon.h"
#include <cmath>

bool IsIdentityMatrix(const Matrix4x4& matrix) {
	const Matrix4x4 identity = Function::MakeIdentity4x4();
	const float epsilon = 1e-5f;
	return std::abs(matrix.m[0][0] - identity.m[0][0]) < epsilon && std::abs(matrix.m[0][1] - identity.m[0][1]) < epsilon && std::abs(matrix.m[0][2] - identity.m[0][2]) < epsilon &&
	       std::abs(matrix.m[0][3] - identity.m[0][3]) < epsilon && std::abs(matrix.m[1][0] - identity.m[1][0]) < epsilon && std::abs(matrix.m[1][1] - identity.m[1][1]) < epsilon &&
	       std::abs(matrix.m[1][2] - identity.m[1][2]) < epsilon && std::abs(matrix.m[1][3] - identity.m[1][3]) < epsilon && std::abs(matrix.m[2][0] - identity.m[2][0]) < epsilon &&
	       std::abs(matrix.m[2][1] - identity.m[2][1]) < epsilon && std::abs(matrix.m[2][2] - identity.m[2][2]) < epsilon && std::abs(matrix.m[2][3] - identity.m[2][3]) < epsilon &&
	       std::abs(matrix.m[3][0] - identity.m[3][0]) < epsilon && std::abs(matrix.m[3][1] - identity.m[3][1]) < epsilon && std::abs(matrix.m[3][2] - identity.m[3][2]) < epsilon &&
	       std::abs(matrix.m[3][3] - identity.m[3][3]) < epsilon;
}
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

Skeleton Skeleton::Create(const Model::Node& rootNode) {
	Skeleton skeleton;
	const Model::Node& baseNode = skeleton.GetRootNode(rootNode);
	skeleton.root_ = CreateJoint(baseNode, std::nullopt, skeleton.joints_);

	for (const Joint& joint : skeleton.joints_) {
		skeleton.jointMap_.emplace(joint.name, joint.index);
	}

	skeleton.Update();
	return skeleton;
}

const Model::Node& Skeleton::GetRootNode(const Model::Node& rootNode) const {
	if (IsIdentityMatrix(rootNode.localMatrix) && rootNode.children.size() == 1) {
		return rootNode.children.front();
	}
	return rootNode;
}

void Skeleton::Update() {
	for (Joint& joint : joints_) {
		joint.localMatrix = Function::MakeAffineMatrix(joint.transform.scale, joint.transform.quaternion, joint.transform.translate);
		if (joint.parent) {
			joint.skeletonSpaceMatrix = Function::Multiply(joint.localMatrix, joints_[*joint.parent].skeletonSpaceMatrix);
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Skeleton::ApplyAnimation(const Animation::AnimationData& animation, float animationTime) {
	for (Joint& joint : joints_) {
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

void Skeleton::SetObjectMatrix(const Matrix4x4& objectMatrix) { objectMatrix_ = objectMatrix; }

Vector3 Skeleton::GetJointWorldPosition(const Joint& joint) const {
	Matrix4x4 worldMatrix = Function::Multiply(joint.skeletonSpaceMatrix, objectMatrix_);
	return Function::TransformVM({0.0f, 0.0f, 0.0f}, worldMatrix);
}

void Skeleton::UpdateAnimation(const Animation::AnimationData& animation, float& animationTime, float deltaTime) {
	if (animation.duration <= 0.0f) {
		Update();
		return;
	}

	animationTime += deltaTime;
	animationTime = std::fmod(animationTime, animation.duration);
	ApplyAnimation(animation, animationTime);
	Update();
}

void Skeleton::DrawBones(Primitive* jointPrimitive, Primitive* bonePrimitive, const Vector4& jointColor, const Vector4& boneColor) const {
	if (!jointPrimitive || !bonePrimitive) {
		return;
	}

	jointPrimitive->SetColor(jointColor);
	bonePrimitive->SetColor(boneColor);
	jointPrimitive->SetEnableLighting(false);
	bonePrimitive->SetEnableLighting(false);

	Object3dCommon::GetInstance()->DrawCommonWireframeNoDepth();
	for (const Joint& joint : joints_) {
		Vector3 jointPosition = GetJointWorldPosition(joint);
		jointPrimitive->SetTransform({
		    .scale{kJointRadius,    kJointRadius,    kJointRadius   },
		    .rotate{0.0f,            0.0f,            0.0f           },
		    .translate{jointPosition.x, jointPosition.y, jointPosition.z},
		});
		jointPrimitive->Update();
		jointPrimitive->Draw();
	}

	Object3dCommon::GetInstance()->DrawCommonLineNoDepth();
	for (const Joint& joint : joints_) {
		if (!joint.parent.has_value()) {
			continue;
		}

		const Joint& parentJoint = joints_[*joint.parent];
		Vector3 jointPosition = GetJointWorldPosition(joint);
		Vector3 parentPosition = GetJointWorldPosition(parentJoint);
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