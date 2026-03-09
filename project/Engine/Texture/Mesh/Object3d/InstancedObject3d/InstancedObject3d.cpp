#include "InstancedObject3d.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "Model/Model.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <numbers>
#include <random>

namespace {
constexpr uint32_t kCoffeeSlices = 10;
constexpr float kCoffeeRadius = 0.03f;
constexpr float kCoffeeHeight = 0.10f;
constexpr float kCoffeeTopY = kCoffeeHeight * 0.5f;
constexpr float kCoffeeBottomY = -kCoffeeHeight * 0.5f;
constexpr float kRoomWidth = 9.0f;
constexpr float kRoomDepth = 15.0f;
constexpr float kSpawnMargin = 0.08f;
constexpr float kCollisionRadius = kCoffeeRadius * 1.4f;
constexpr uint32_t kCollisionThreadGroupSize = 64;
constexpr uint32_t kCollisionRelaxationIterations = 2;

struct CollisionParams {
	uint32_t instanceCount;
	float minDistance;
	float roomMinX;
	float roomMaxX;
	float roomMinZ;
	float roomMaxZ;
	float floorY;
	float pad0;
};

} // namespace

void InstancedObject3d::CreateDefaultMesh() {
	coffeeVertices_.clear();
	coffeeIndices_.clear();
	for (uint32_t i = 0; i < kCoffeeSlices; ++i) {
		const float t = (static_cast<float>(i) / static_cast<float>(kCoffeeSlices)) * (std::numbers::pi_v<float> * 2.0f);
		const float x = std::cos(t) * kCoffeeRadius;
		const float z = std::sin(t) * kCoffeeRadius;
		const Vector3 sideNormal = Function::Normalize({x, 0.0f, z});
		coffeeVertices_.push_back({
		    {x, kCoffeeTopY, z},
            sideNormal
        });
		coffeeVertices_.push_back({
		    {x, kCoffeeBottomY, z},
            sideNormal
        });
	}
	const uint32_t topCenterIndex = static_cast<uint32_t>(coffeeVertices_.size());
	coffeeVertices_.push_back({
	    {0.0f, kCoffeeTopY, 0.0f},
        {0.0f, 1.0f,        0.0f}
    });
	const uint32_t bottomCenterIndex = static_cast<uint32_t>(coffeeVertices_.size());
	coffeeVertices_.push_back({
	    {0.0f, kCoffeeBottomY, 0.0f},
        {0.0f, -1.0f,          0.0f}
    });

	for (uint32_t i = 0; i < kCoffeeSlices; ++i) {
		const uint32_t next = (i + 1) % kCoffeeSlices;
		const uint32_t top0 = i * 2;
		const uint32_t bottom0 = i * 2 + 1;
		const uint32_t top1 = next * 2;
		const uint32_t bottom1 = next * 2 + 1;
		coffeeIndices_.insert(coffeeIndices_.end(), {top0, top1, bottom0, bottom0, top1, bottom1, topCenterIndex, top0, top1, bottomCenterIndex, bottom1, bottom0});
	}
}

void InstancedObject3d::CreateMeshFromModel(const Model& model) {
	coffeeVertices_.clear();
	coffeeIndices_.clear();

	const auto& modelData = model.GetModelData();
	coffeeVertices_.reserve(modelData.vertices.size());
	for (const auto& vertex : modelData.vertices) {
		coffeeVertices_.push_back({
		    {vertex.position.x, vertex.position.y, vertex.position.z},
		    vertex.normal,
		});
	}

	coffeeIndices_ = modelData.indices;
	if (coffeeIndices_.empty()) {
		coffeeIndices_.resize(coffeeVertices_.size());
		for (uint32_t i = 0; i < coffeeIndices_.size(); ++i) {
			coffeeIndices_[i] = i;
		}
	}
}

void InstancedObject3d::CreateMesh() {
	if (modelPath_.empty()) {
		CreateDefaultMesh();
		return;
	}

	std::unique_ptr<Model> modelInstance = ModelManager::GetInstance()->CreateModelInstance(modelPath_);
	if (modelInstance) {
		CreateMeshFromModel(*modelInstance);
		return;
	}

	Model* model = ModelManager::GetInstance()->FindModel(modelPath_);
	if (model) {
		CreateMeshFromModel(*model);
		return;
	}

	CreateDefaultMesh();
}

void InstancedObject3d::CreateInstancingPipeline() {
	const std::wstring vsPath = L"Resources/shader/Object3d/VS_Shader/InstancedObject3d.VS.hlsl";
	const std::wstring psPath = L"Resources/shader/Object3d/PS_Shader/InstancedObject3d.PS.hlsl";
	pso_ = std::make_unique<CreatePSO>(Object3dCommon::GetInstance()->GetDxCommon());
	pso_->CreateInstancedObject3d(psPath, vsPath);
}

void InstancedObject3d::CreateCollisionPipeline() {
	auto* dxCommon = Object3dCommon::GetInstance()->GetDxCommon();
	D3D12_DESCRIPTOR_RANGE uavRanges[1] = {};
	uavRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	uavRanges[0].NumDescriptors = 2;
	uavRanges[0].BaseShaderRegister = 0;
	uavRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = uavRanges;
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));

	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&collisionRootSignature_));
	assert(SUCCEEDED(hr));

	auto csBlob = dxCommon->CompileShader(L"Resources/shader/Object3d/CS_Shader/InstancedObject3dCollision.CS.hlsl", L"cs_6_0");
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineDesc{};
	computePipelineDesc.pRootSignature = collisionRootSignature_.Get();
	computePipelineDesc.CS = {csBlob->GetBufferPointer(), csBlob->GetBufferSize()};
	hr = dxCommon->GetDevice()->CreateComputePipelineState(&computePipelineDesc, IID_PPV_ARGS(&collisionPipelineState_));
	assert(SUCCEEDED(hr));
}

void InstancedObject3d::CreateBuffers() {
	auto* object3dCommon = Object3dCommon::GetInstance();
	auto* srvManager = TextureManager::GetInstance()->GetSrvManager();

	vertexResource_ = object3dCommon->CreateBufferResource(sizeof(CoffeeVertex) * coffeeVertices_.size());
	CoffeeVertex* mappedVertex = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertex));
	std::memcpy(mappedVertex, coffeeVertices_.data(), sizeof(CoffeeVertex) * coffeeVertices_.size());
	vertexBufferView_ = {vertexResource_->GetGPUVirtualAddress(), static_cast<UINT>(sizeof(CoffeeVertex) * coffeeVertices_.size()), sizeof(CoffeeVertex)};

	indexResource_ = object3dCommon->CreateBufferResource(sizeof(uint32_t) * coffeeIndices_.size());
	uint32_t* mappedIndex = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndex));
	std::memcpy(mappedIndex, coffeeIndices_.data(), sizeof(uint32_t) * coffeeIndices_.size());
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * coffeeIndices_.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	instanceResource_ = object3dCommon->CreateBufferResource(sizeof(Matrix4x4) * kCoffeeInstanceCount_);
	instanceDataResource_ = object3dCommon->CreateBufferResource(sizeof(InstanceData) * kCoffeeInstanceCount_);
	InstanceData* mappedInstanceData = nullptr;
	instanceDataResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedInstanceData));

	const float minX = -kRoomWidth * 0.5f + kSpawnMargin;
	const float maxX = kRoomWidth * 0.5f - kSpawnMargin;
	const float minZ = -kRoomDepth * 0.5f + kSpawnMargin;
	const float maxZ = kRoomDepth * 0.5f - kSpawnMargin;

	std::mt19937 randomEngine(20250518u);
	std::uniform_real_distribution<float> posXDist(minX, maxX);
	std::uniform_real_distribution<float> posZDist(minZ, maxZ);
	std::uniform_real_distribution<float> rotYDist(0.0f, std::numbers::pi_v<float> * 2.0f);
	std::uniform_real_distribution<float> scaleDist(0.75f, 1.15f);

	for (uint32_t i = 0; i < kCoffeeInstanceCount_; ++i) {
		mappedInstanceData[i].position = {posXDist(randomEngine), kCollisionRadius, posZDist(randomEngine)};
		mappedInstanceData[i].yaw = rotYDist(randomEngine);
		mappedInstanceData[i].scale = scaleDist(randomEngine);
		mappedInstanceData[i].padding = {0.0f, 0.0f, 0.0f};
	}

	if (!hasInstanceSrvIndex_) {
		instanceSrvIndex_ = srvManager->Allocate();
		instanceDataUavIndex_ = srvManager->Allocate();
		instanceWorldUavIndex_ = srvManager->Allocate();
		hasInstanceSrvIndex_ = true;
	}
	srvManager->CreateSRVforStructuredBuffer(instanceSrvIndex_, instanceResource_.Get(), kCoffeeInstanceCount_, sizeof(Matrix4x4));
	srvManager->CreateUAVforStructuredBuffer(instanceDataUavIndex_, instanceDataResource_.Get(), kCoffeeInstanceCount_, sizeof(InstanceData));
	srvManager->CreateUAVforStructuredBuffer(instanceWorldUavIndex_, instanceResource_.Get(), kCoffeeInstanceCount_, sizeof(Matrix4x4));

	if (!hasCollisionDescriptor_) {
		collisionParamResource_ = object3dCommon->CreateBufferResource(sizeof(CollisionParams));
		CollisionParams* params = nullptr;
		collisionParamResource_->Map(0, nullptr, reinterpret_cast<void**>(&params));
		params->instanceCount = kCoffeeInstanceCount_;
		params->minDistance = kCollisionRadius * 2.0f;
		params->roomMinX = minX;
		params->roomMaxX = maxX;
		params->roomMinZ = minZ;
		params->roomMaxZ = maxZ;
		params->floorY = kCollisionRadius;
		params->pad0 = 0.0f;
		hasCollisionDescriptor_ = true;
	}

	sceneConstantResource_ = object3dCommon->CreateBufferResource(sizeof(CoffeeSceneConstants));
	sceneConstantResource_->Map(0, nullptr, reinterpret_cast<void**>(&sceneConstants_));
	sceneConstants_->viewProjection = Function::MakeIdentity4x4();
	sceneConstants_->lightDirection = {0.1f, -0.5f, -0.2f, 1.0f};
}

void InstancedObject3d::DispatchCollision() {
	auto* dxCommon = Object3dCommon::GetInstance()->GetDxCommon();
	auto* srvManager = TextureManager::GetInstance()->GetSrvManager();
	auto* commandList = dxCommon->GetCommandList();

	D3D12_RESOURCE_BARRIER toUavBarriers[2] = {};
	toUavBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	toUavBarriers[0].Transition.pResource = instanceDataResource_.Get();
	toUavBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	toUavBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	toUavBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	toUavBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	toUavBarriers[1].Transition.pResource = instanceResource_.Get();
	toUavBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	toUavBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	toUavBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(2, toUavBarriers);

	ID3D12DescriptorHeap* heaps[] = {srvManager->GetDescriptorHeap().Get()};
	commandList->SetDescriptorHeaps(_countof(heaps), heaps);
	commandList->SetPipelineState(collisionPipelineState_.Get());
	commandList->SetComputeRootSignature(collisionRootSignature_.Get());
	commandList->SetComputeRootDescriptorTable(0, srvManager->GetGPUDescriptorHandle(instanceDataUavIndex_));
	commandList->SetComputeRootConstantBufferView(1, collisionParamResource_->GetGPUVirtualAddress());

	const uint32_t dispatchCount = (kCoffeeInstanceCount_ + kCollisionThreadGroupSize - 1u) / kCollisionThreadGroupSize;
	for (uint32_t i = 0; i < kCollisionRelaxationIterations; ++i) {
		commandList->Dispatch(dispatchCount, 1, 1);
		D3D12_RESOURCE_BARRIER uavBarrier[2] = {};
		uavBarrier[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier[0].UAV.pResource = instanceDataResource_.Get();
		uavBarrier[1].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier[1].UAV.pResource = instanceResource_.Get();
		commandList->ResourceBarrier(2, uavBarrier);
	}

	D3D12_RESOURCE_BARRIER toReadBarriers[2] = {};
	toReadBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	toReadBarriers[0].Transition.pResource = instanceDataResource_.Get();
	toReadBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	toReadBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	toReadBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	toReadBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	toReadBarriers[1].Transition.pResource = instanceResource_.Get();
	toReadBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	toReadBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	toReadBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(2, toReadBarriers);
}

void InstancedObject3d::Initialize(const std::string& modelPath) {
	modelPath_ = modelPath;
	CreateMesh();
	CreateInstancingPipeline();
	CreateCollisionPipeline();
	CreateBuffers();
	isInitialized_ = true;
}

void InstancedObject3d::SetModel(const std::string& modelPath) {
	modelPath_ = modelPath;
	if (!isInitialized_) {
		return;
	}
	CreateMesh();
	CreateBuffers();
}

void InstancedObject3d::Update(const Camera* camera, const Vector3& lightDirection) {
	sceneConstants_->viewProjection = camera->GetViewProjectionMatrix();
	sceneConstants_->lightDirection = {lightDirection.x, lightDirection.y, lightDirection.z, 1.0f};
	DispatchCollision();
}

void InstancedObject3d::Draw() {
	auto* dxCommon = Object3dCommon::GetInstance()->GetDxCommon();
	auto* commandList = dxCommon->GetCommandList();
	auto* srvManager = TextureManager::GetInstance()->GetSrvManager();
	ID3D12DescriptorHeap* descriptorHeaps[] = {srvManager->GetDescriptorHeap().Get()};
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	commandList->SetGraphicsRootSignature(pso_->GetRootSignature().Get());
	commandList->SetPipelineState(pso_->GetGraphicsPipelineState(BlendMode::kBlendModeNone).Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	srvManager->SetGraphicsRootDescriptorTable(0, instanceSrvIndex_);
	commandList->SetGraphicsRootConstantBufferView(1, sceneConstantResource_->GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(static_cast<UINT>(coffeeIndices_.size()), kCoffeeInstanceCount_, 0, 0, 0);
}