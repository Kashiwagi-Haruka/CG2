#include "SkinCluster.h"
#include "Function.h"
#include "Logger.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include <algorithm>
#include <cassert>
#include <cstring>

SkinCluster CreateSkinCluster(ModelCommon* modelCommon, const Skeleton& skeleton, const Model& model) {
	SkinCluster skinCluster{};
	if (!modelCommon) {
		Logger::Log("CreateSkinCluster failed: ModelCommon is null.\n");
		return skinCluster;
	}

	const auto& joints = skeleton.GetJoints();
	if (joints.empty()) {
		Logger::Log("CreateSkinCluster skipped: skeleton has no joints.\n");
		return skinCluster;
	}

	const auto& modelData = model.GetModelData();

	// palette用Resourceを確保
	skinCluster.paletteResource = modelCommon->CreateBufferResource(sizeof(WellForGPU) * joints.size());
	WellForGPU* mappedPalette = nullptr;
	skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
	skinCluster.mappedPalette = {mappedPalette, joints.size()};

	// palette用SRVを作成
	auto* srvManager = TextureManager::GetInstance()->GetSrvManager();
	assert(srvManager->CanAllocate());
	skinCluster.paletteSrvIndex = srvManager->Allocate();
	skinCluster.paletteSrvHandle.first = srvManager->GetCPUDescriptorHandle(skinCluster.paletteSrvIndex);
	skinCluster.paletteSrvHandle.second = srvManager->GetGPUDescriptorHandle(skinCluster.paletteSrvIndex);
	srvManager->CreateSRVforStructuredBuffer(skinCluster.paletteSrvIndex, skinCluster.paletteResource.Get(), static_cast<UINT>(joints.size()), sizeof(WellForGPU));

	// Influence用Resourceを確保
	skinCluster.influenceResource = modelCommon->CreateBufferResource(sizeof(VertexInfluence) * modelData.vertices.size());
	VertexInfluence* mappedInfluence = nullptr;
	skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
	std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData.vertices.size());
	skinCluster.mappedInfluence = {mappedInfluence, modelData.vertices.size()};

	// Influence用VBVを作成
	skinCluster.influenceBufferView.BufferLocation = skinCluster.influenceResource->GetGPUVirtualAddress();
	skinCluster.influenceBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexInfluence) * modelData.vertices.size());
	skinCluster.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

	// InverseBindPoseMatrixを格納する場所を作成
	skinCluster.inverseBindPoseMatrices.resize(joints.size());
	std::generate(skinCluster.inverseBindPoseMatrices.begin(), skinCluster.inverseBindPoseMatrices.end(), Function::MakeIdentity4x4);

	// ModelDataのSkinCluster情報を解析してInfluenceの中身を埋める
	for (const auto& [jointName, jointWeight] : modelData.skinClusterData) {
		auto jointIndex = skeleton.FindJointIndex(jointName);
		if (!jointIndex) {
			continue;
		}

		skinCluster.inverseBindPoseMatrices[*jointIndex] = jointWeight.inverseBindPoseMatrix;
		for (const auto& vertexWeight : jointWeight.vertexWeights) {
			auto& currentInfluence = skinCluster.mappedInfluence[vertexWeight.vertexIndex];
			for (uint32_t index = 0; index < kNumMaxInfluence; ++index) {
				if (currentInfluence.weights[index] == 0.0f) {
					currentInfluence.weights[index] = vertexWeight.weight;
					currentInfluence.jointIndices[index] = *jointIndex;
					break;
				}
			}
		}
	}

	return skinCluster;
}

void UpdateSkinCluster(SkinCluster& skinCluster, const Skeleton& skeleton) {
	const auto& joints = skeleton.GetJoints();
	for (size_t jointIndex = 0; jointIndex < joints.size(); ++jointIndex) {
		assert(jointIndex < skinCluster.inverseBindPoseMatrices.size());
		skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix = Function::Multiply(skinCluster.inverseBindPoseMatrices[jointIndex], joints[jointIndex].skeletonSpaceMatrix);
		skinCluster.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix = Function::Transpose(Function::Inverse(skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix));
	}
}